
/*
 */

#ifndef _feutil_h_
#define _feutil_h_

char *decodeTone(fe_tone_t tone);

fe_polarity_t encodePolar(char *polar);
char *decodePolar(fe_polarity_t polar);

fe_inner_t encodeFEC(char *inner);
char *decodeFEC(fe_inner_t inner);

#endif
