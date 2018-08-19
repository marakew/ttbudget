#include <stdlib.h>
#include <stdio.h>
//#include <string.h>

#include "getopt.h"

char *optarg = NULL;
int optind = 0;
int opterr = 1;
int optopt = '?';

static char *nextchar;

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

static char *posixly_correct;
static int first_nonopt;
static int last_nonopt;

//
static char *nonoption_flags;
static int nonoption_flags_len;

static char *
my_index(const char *str, int chr)
{
	while(*str)
	{
		if (*str == chr)
			return (char *) str;
		str++;
	}
	return 0;
}

# define SWAP_FLAGS(ch1, ch2)

static void
exchange(char **argv)
{
	int bottom = first_nonopt;
	int middle = last_nonopt;
	int top = optind;
	char *tem;

	while (top > middle && middle > bottom){

		if (top - middle > middle - bottom){

			int len = middle - bottom;
			register int i;

			for (i = 0; i < len; i++){
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
				SWAP_FLAGS (bottom + i, top - (middle - bottom) + i);
			}
		
			top -= len;
		} else {
			int len = top - middle;
			register int i;

			for (i = 0; i < len; i++){
				tem = argv[bottom + i];
				argv[bottom + i] = argv[middle + i];
				argv[middle + i] = tem;
				SWAP_FLAGS (bottom + i, middle + i);
			}
			bottom += len;
		}
	}

	first_nonopt += (optind - last_nonopt);
	last_nonopt = optind;
}


static const char *
_getopt_initialize(int argc, char *const *argv, const char *shortopts)
{
	optind = 1;
	first_nonopt = last_nonopt = 1;
	nextchar = NULL;
	posixly_correct = getenv("POSIXLY_CORRECT");

	if (shortopts[0] == '-'){
		ordering = RETURN_IN_ORDER;
		++shortopts;
	} else
	if (shortopts[0] == '+'){
		ordering = REQUIRE_ORDER;
		++shortopts;
	} else
	if (posixly_correct != NULL)
		ordering = REQUIRE_ORDER;
	else
		ordering = PERMUTE;

	if (posixly_correct == NULL){
		char var[40];
		sprintf(var, "_%d_GNU_nonoption_argv_flags_", getpid());
		nonoption_flags = getenv(var);
		if (nonoption_flags == NULL)
			nonoption_flags_len = 0;
		else
			nonoption_flags_len = strlen(var); //nonoption_flags
	} else
		nonoption_flags_len = 0;

	return shortopts;
}


int
_getopt_internal(int argc, char *const *argv, const char *shortopts,
	const struct option *longopts, int *longind, int long_only)
{
	optarg = NULL;

	if (optind == 0){
		shortopts = _getopt_initialize(argc, argv, shortopts);
		optind = 1;
	}

#if 1
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0'		\
			|| (optind < nonoption_flags_len			\
			&& nonoption_flags[optind] == '1'))
#else
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')
#endif
	if (nextchar == NULL || *nextchar == '\0'){

		if (last_nonopt > optind)
			last_nonopt = optind;

		if (first_nonopt > optind)
			first_nonopt = optind;

		if (ordering == PERMUTE){

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **)argv);
			else
			if (last_nonopt != optind)
				first_nonopt = optind;

			while (optind < argc && NONOPTION_P)
				optind++;

			last_nonopt = optind;
		}


		if (optind != argc && !strcmp(argv[optind], "--")){
			optind++;

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange((char **)argv);
			else
			if (first_nonopt == last_nonopt)
				first_nonopt = optind;

			last_nonopt = argc;

			optind = argc;
		}

		if (optind == argc){
			if (first_nonopt != last_nonopt)
				optind = first_nonopt;
			return -1;
		}

		if (NONOPTION_P){
			if (ordering == REQUIRE_ORDER)
				return -1;
			optarg = argv[optind++];
			return 1;
		}

		nextchar = (argv[optind] + 1
			+ (longopts != NULL && argv[optind][1] == '-'));
	}


	if (longopts != NULL &&
		(argv[optind][1] == '-' || (long_only &&
		(argv[optind][2] || !my_index(shortopts, argv[optind][1]) )))){

		char *nameend;
		const struct option *p;
		const struct option *pfound = NULL;
		int exact = 0;
		int ambig = 0;
		int indfound = -1;
		int option_index;

		for (nameend = nextchar; *nameend && *nameend != '='; nameend++);

		for (p = longopts, option_index = 0; p->name; p++, option_index++){
			if (!strncmp(p->name, nextchar, nameend - nextchar)){

				if ((unsigned int)(nameend - nextchar)
					== (unsigned int)strlen(p->name)){
					pfound = p;
					indfound = option_index;
					exact = 1;
					break;
				} else
				if (pfound == NULL){
					pfound = p;
					indfound = option_index;
				} else
					ambig = 1;
			}
		}
		if (ambig && !exact){

			if (opterr)
				fprintf(stderr, ("%s: option `%s' is ambiguous\n"),
					argv[0], argv[optind]);

			nextchar += strlen(nextchar);
			optind++;
			optopt = 0;
			return '?';
		}

		if (pfound != NULL){
			option_index = indfound;
			optind++;

			if (*nameend){
				if (pfound->has_arg)
					optarg = nameend + 1;
				else {
					if (opterr){
						if (argv[optind - 1][1] == '-')
							fprintf(stderr, ("%s: option `--%s' doesn't allow an argument\n"),
								argv[0], pfound->name);
						else
							fprintf(stderr, ("%s: option `%c%s' doesn't allow an argument\n"),
								argv[0], argv[optind - 1][0], pfound->name);

						nextchar += strlen(nextchar);
						optopt = pfound->val;
						return '?';
					}
				}
			} else
			if (pfound->has_arg == 1){
				if (optind < argc)
					optarg = argv[optind++];
				else {
					if (opterr)
						fprintf(stderr, ("%s: option `%s' requires an argument\n"),
							argv[0], argv[optind - 1]);
					nextchar += strlen(nextchar);
					optopt = pfound->val;
					return shortopts[0] == ':' ? ':' : '?';
				}
			}

			nextchar += strlen(nextchar);
			if (longind != NULL)
				*longind = option_index;

			if (pfound->flag){
				*(pfound->flag) = pfound->val;
				return 0;
			}

			return pfound->val;
		} 

		if (!long_only || argv[optind][1] == '-'
		|| my_index(shortopts, *nextchar) == NULL){
			if (opterr){
				if (argv[optind][1] == '-')
					fprintf(stderr, ("%s: unrecognized option `--%s'\n"),
						argv[0], nextchar);
				else
					fprintf(stderr, ("%s: unrecognized option `%c%s'\n"),
						argv[0], argv[optind][0], nextchar);
			}
			nextchar = (char *)"";
			optind++;
			optopt = 0;
			return '?';
		}
	}

  /* Look at and handle the next short option-character.  */
  {
	char c = *nextchar++;
	char *temp = my_index(shortopts, c);

	if (*nextchar == '\0')
		++optind;

	if (temp == NULL || c == ':'){
		if (opterr){
			if (posixly_correct)
				fprintf(stderr,("%s: illegal option -- %c\n"),
					argv[0], c);
			else
				fprintf(stderr,("%s: invalid option -- %c\n"),
					argv[0], c);
		}
		optopt = c;
		return '?';
	}

	if (temp[1] == ':'){
		if (temp[2] == ':'){
			if (*nextchar != '\0'){
				optarg = nextchar;
				optind++;
			} else
				optarg = NULL;
			nextchar = NULL;
		} else {

			if (*nextchar != '\0'){
				optarg = nextchar;
				optind++;
			} else
			if (optind == argc){
				if (opterr){
					fprintf(stderr, ("%s: option requires an argument -- %c\n"),
						argv[0], c);
				}
				optopt = c;
				if (shortopts[0] == ':')
					c = ':';
				else
					c = '?';
			} else
				optarg = argv[optind++];
			nextchar = NULL;
		}
	}
	return c;
  }
}

int
getopt(int argc, char *const *argv, const char *shortopts)
{
	return _getopt_internal(argc, argv, shortopts,
			(const struct option *)0, (int *)0, 0);
}

int
getopt_long(int argc, char *const *argv, const char *shortopts,
	const struct option *longopts, int *opt_index)
{
	return _getopt_internal(argc, argv, shortopts,
				longopts, opt_index, 0);
}

int
getopt_long_only(int argc, char *const *argv, const char *shortopts,
	const struct option *longopts, int *opt_index)
{
	return _getopt_internal(argc, argv, shortopts,
				longopts, opt_index, 1);
}

