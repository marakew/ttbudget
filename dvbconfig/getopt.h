#ifndef _getopt_h_
#define _getopt_h_

#ifdef	__cplusplus
extern "C" {
#endif

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

struct option {
	char	*name;
	int	has_arg;
	int	*flag;
	int	val;
};

#define	no_argument		0
#define required_argument	1
#define optional_argument	2

extern int
getopt(int argc, char *const *argv, const char *shortopts);

extern int
getopt_long(int argc, char *const *argv, const char *shortopts,
		const struct option *longopts, int *longind);
extern int
getopt_long_only(int argc, char *const *argv, const char *shortopts,
		const struct option *longopts, int *longind);
extern int
_getopt_internal(int argc, char *const *argv, const char *shortopts,
		const struct option *longopts, int *longind, int long_only);

#ifdef	__cplusplus
}
#endif

#endif /* getopt.h */
