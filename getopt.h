
#define LOCAL_GETOPT

extern int optind, optopt;
extern char *optarg;

extern int getopt(int nargc, char *nargv[], char *ostr);
