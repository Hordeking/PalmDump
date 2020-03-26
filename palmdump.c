/*

		           P A L M D U M P
		           ===============

		   Dump a Palm OS .pdb or .prc file.

		           by John Walker
		      http://www.fourmilab.ch/
		This program is in the public domain

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "pdb.h"
#include "palmread.h"

#ifdef _WIN32
	#include <getopt.h>
#else
	#include <unistd.h>
#endif

extern void xd(FILE *out, unsigned char *buf, int bufl, int dochar);

static FILE * fi;

/*Extract a 4 character string jammed into a long. */
#define stringAlong(s, l)\
s[0] = (char)(l >> 24);\
s[1] = (char)(l >> 16);\
s[2] = (char)(l >> 8);\
s[3] = (char)(l);\
s[4] = 0

/*palmDate -- Edit Palm date and time, checking for invalid
	dates and erroneous PC/Unix style dates. */

static char *
	palmDate(unsigned long t)
	{
		struct tm * utm;
		time_t ut = (time_t) t;
		static char s[132];
		static char invalid[] = "Invalid (%lu)\n";

		/*Microsoft marching morons return NULL from localtime() if
			passed a date before the epoch, thus planting a Y2038
			bomb in every program that uses the function. Just in
			case, we test for NULL return and say "Invalid" if so. */

		if (ut < timeOffset)
		{ /*This looks like a PC/Unix style date */
			utm = localtime(&ut);

			if (utm == NULL)
			{
				sprintf(s, invalid, t);
				return s;
			}

			sprintf(s, "PC/Unix time: %s", asctime(utm));
			return s;
		}

		/*Otherwise assume it is a Palm/Macintosh style date */

		ut = ((unsigned long) ut) - timeOffset;
		utm = localtime(&ut);
		if (utm == NULL)
		{
			sprintf(s, invalid, t);
			return s;
		}
		sprintf(s, "Palm time: %s", asctime(utm));
		return s;
	}

/*USAGE -- Print how-to-call information. */

static void usage(void)
{
	fprintf(stderr, "palmdump[options] ifile[ofile] \n");
	fprintf(stderr, "Dump Palm Computing® Platform PDB or PRC file.\n");
	fprintf(stderr, "   Options:\n");
	fprintf(stderr, "       -n             Dump record/resource headers, but not content\n");
	fprintf(stderr, "       -u             Print this message\n");
	fprintf(stderr, "Originally by John Walker (http://www.fourmilab.ch/)\n");
	fprintf(stderr, "This program is in the public domain.\n");
}

/*Main program. */

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	FILE *fo = stdout;

	int i, opt, content = 1;
	struct palmreadContext * rc;
	PDBHeader ph;
	char *inname;
	char dbname[kMaxPDBNameSize + 2];

	while ((opt = getopt(argc, argv, "nu")) != -1)
	{
		switch (opt)
		{
			case 'n':
				content = 0;
				break;

			case 'u':
				usage();
				return 0;

			default:
				usage();
				return 2;
		}
	}

	i = 0;

	for (; optind < argc; optind++)
	{
		switch (i)
		{
			case 0:
				inname = argv[optind];
				break;

			case 1:
				fo = fopen(argv[optind], "w");
				if (fo == NULL)
				{
					fprintf(stderr, "Cannot create output file %s.\n", argv[optind]);
					return 2;
				}
				break;

			case 2:
				fprintf(stderr, "Too many file names specified.\n");
				usage();
				return 2;
		}
		i++;
	}

	/*Error if no input file name specified. */

	if (i == 0)
	{
		fprintf(stderr, "No input file name specified.\n");
		usage();
		return 2;
	}

	fi = fopen(inname, "rb");
	if (fi == NULL)
	{
		fprintf(stderr, "Cannot open input file %s.\n", inname);
		return 2;
	}

	rc = palmReadHeader(fi, &ph);

	/*Dump header. */

	memcpy(dbname, ph.name, kMaxPDBNameSize);
	dbname[kMaxPDBNameSize] = 0; /*Guarantee null terminated */
	fprintf(fo, "Database name:           %s\n", dbname);
	fprintf(fo, "Flags:                   0x%X", ph.flags);
	if (ph.flags &pdbOpenFlag)
	{
		fprintf(fo, " Open");
	}
	if (ph.flags &pdbStream)
	{
		fprintf(fo, " Stream");
	}
	if (ph.flags &pdbResetAfterInstall)
	{
		fprintf(fo, " ResetAfterInstall");
	}
	if (ph.flags &pdbOKToInstallNewer)
	{
		fprintf(fo, " InstallNewerOK");
	}
	if (ph.flags &pdbBackupFlag)
	{
		fprintf(fo, " Backup");
	}
	if (ph.flags &pdbAppInfoDirtyFlag)
	{
		fprintf(fo, " AppInfoDirty");
	}
	if (ph.flags &pdbReadOnlyFlag)
	{
		fprintf(fo, " Read-only");
	}
	if (ph.flags &pdbResourceFlag)
	{
		fprintf(fo, " Resource");
	}
	fprintf(fo, "\n");
	fprintf(fo, "Version:                 0x%X", ph.version);
	if (ph.version &pdbVerShowSecret)
	{
		fprintf(fo, " ShowSecret");
	}
	if (ph.version &pdbVerExclusive)
	{
		fprintf(fo, " Exclusive");
	}
	if (ph.version &pdbVerLeaveOpen)
	{
		fprintf(fo, " LeaveOpen");
	}
	if ((ph.version &3) == pdbVerReadWrite)
	{
		fprintf(fo, " Read/write");
	}
	if ((ph.version &3) == pdbVerWrite)
	{
		fprintf(fo, " Write");
	}
	if ((ph.version &3) == pdbVerReadOnly)
	{
		fprintf(fo, " Read-only");
	}
	fprintf(fo, "\n");

	fprintf(fo, "Creation time:           %s", palmDate(ph.creationTime));
	fprintf(fo, "Modification time:       %s", palmDate(ph.modificationTime));
	fprintf(fo, "Backup time:             %s", ph.backupTime == 0 ? "Never\n" : palmDate(ph.backupTime));
	fprintf(fo, "Modification number:     %lu\n", ph.modificationNumber);
	fprintf(fo, "Application Info offset: %lu\n", ph.appInfoOffset);
	fprintf(fo, "Sort Info offset:        %lu\n", ph.sortInfoOffset);

	stringAlong(dbname, ph.type);
	fprintf(fo, "Type:                    %s\n", dbname);

	stringAlong(dbname, ph.creator);
	fprintf(fo, "Creator:                 %s\n", dbname);
	fprintf(fo, "Unique ID:               %lu\n", ph.uniqueID);
	fprintf(fo, "Next record ID:          %lu\n", ph.nextRecordID);
	fprintf(fo, "Number of records:       %u\n", ph.numRecords);

	/*Dump records or resources. */

	if (rc != NULL)
	{
		for (i = 0; i < ph.numRecords; i++)
		{
			unsigned char *rdata;
			long rlen;

			if (rc->isResource)
			{
				PDBResourceEntry re;

				rdata = palmReadResource(rc, i, &re, &rlen);
				if (rdata != NULL)
				{
					char s[5];

					stringAlong(s, re.type);
					fprintf(fo, "\nResource %d:  Length %ld, Offset = %ld, Type \"%s\", ID = %d\n",
						i, rlen, rc->rsp[i].offset, s, re.id);
					if (content)
					{
						xd(fo, rdata, rlen, 1);
					}
					free(rdata);
				}
				else
				{
					fprintf(fo, "\nCannot load resource %d.\n", i);
				}
			}
			else
			{
				PDBRecordEntry re;

				rdata = palmReadRecord(rc, i, &re, &rlen);
				if (rdata != NULL)
				{
					fprintf(fo, "\nRecord %d:  Length %ld, Offset = %ld, Attributes 0x%X", i, rlen, rc->rcp[i].offset, re.attr);
					if (re.attr &dmRecAttrDelete)
					{
						fprintf(fo, " Deleted");
					}
					if (re.attr &dmRecAttrDirty)
					{
						fprintf(fo, " Dirty");
					}
					if (re.attr &dmRecAttrBusy)
					{
						fprintf(fo, " Busy");
					}
					if (re.attr &dmRecAttrSecret)
					{
						fprintf(fo, " Secret");
					}
					fprintf(fo, ", Category %d, UniqueID %d\n",
						re.attr &dmRecAttrCategoryMask, re.uniqueID);
					if (content)
					{
						xd(fo, rdata, rlen, 1);
					}
					free(rdata);
				}
				else
				{
					fprintf(fo, "\nCannot load resource %d.\n", i);
				}
			}
		}
		palmDisposeContext(rc);
	}
	else
	{
		fprintf(stderr, "Cannot read Palm file header.\n");
		return 1;
	}

	fclose(fi);

	return 0;
}
