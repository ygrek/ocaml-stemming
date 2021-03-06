
#include <stdlib.h>  /* for malloc, free */
#include <ctype.h>   /* for tolower */
#include <string.h>  /* for memcmp, memmove */

/*  A direct encoding of the English (Porter2) stemmer defined at

    http://snowball.tartarus.org/algorithms/english/stemmer.html

    -- see this webpage for an explanation.

*/

#include "porter2_c.h"

/* The main part of the stemming algorithm starts here.
*/

#define TRUE 1
#define FALSE 0

/* stemmer is a structure for a few local bits of data,
*/

struct stemmer {
    char * b;     /* buffer for word to be stemmed */
    int k;        /* offset to the end of the string */
    int j;        /* a general offset into the string */
    int p1;
    int p2;
};

/* Member b is a buffer holding a word to be stemmed. The letters are in
   b[0], b[1] ... ending at b[z->k]. Member k is readjusted downwards as
   the stemming progresses. Zero termination is not used in the
   algorithm.

   Typical usage is:

       struct stemmer * z = create_stemmer();
       char b[] = "pencils";
       int res = stem(z, b, 6);
           /- stem the 7 characters of b[0] to b[6]. The result, res,
              will be 5 (the 's' is removed). -/
       free_stemmer(z);
*/

extern struct stemmer * create_stemmer(void) {
    return (struct stemmer *) malloc(sizeof(struct stemmer));
    /* assume malloc succeeds */
}

extern void free_stemmer(struct stemmer * z) {
    free(z);
}

static int cons(int ch) {
    switch (ch) {
        case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
            return FALSE;
   }
   return TRUE;
}

#define vowel(ch) (!cons(ch))

static int vowelinstem(struct stemmer * z) {
    char * b = z->b;
    int j = z->j;
    int i; for (i = 0; i <= j; i++) if (vowel(b[i])) return TRUE;
    return FALSE;
}

static int doublec(struct stemmer * z, int j) {
    char * b = z->b;
    if (j < 1) return FALSE;
    if (b[j] == b[j - 1]) switch (b[j]) {
        case 'b': case 'd': case 'f': case 'g': case 'm': case 'n': case 'p':
        case 'r': case 't':
            return TRUE;
    }
    return FALSE;
}

static int valid_li_ending(struct stemmer * z) {
    if (z->j == 0) return FALSE;
    switch (z->b[z->j]) {
        case 'c': case 'd': case 'e': case 'g': case 'h': case 'k': case 'm':
        case 'n': case 'r': case 't':
            return TRUE;
    }
    return FALSE;
}

static int shortv(struct stemmer * z) {
    char * b = z->b;
    int i = z->j;
    int ch = b[i];
    if vowel(ch) return FALSE;
    if (i == 1 && vowel(b[0])) return TRUE;
    if (i > 1 && vowel(b[i - 1]) && cons(b[i - 2]) && ch != 'w' && ch != 'x' && ch != 'Y')
        return TRUE;
    return FALSE;
}

static int ends(struct stemmer * z, char * s) {
    int length = s[0];
    char * b = z->b;
    int k = z->k;
    if (s[length] != b[k]) return FALSE; /* tiny speed-up */
    if (length > k + 1) return FALSE;
    if (memcmp(b + k - length + 1, s + 1, length) != 0) return FALSE;
    z->j = k-length;
    return TRUE;
}

static int equals(struct stemmer * z, char * s) {
    if (s[0] != z->k + 1) return FALSE;
    return ends(z, s);
}

static int starts(struct stemmer * z, char * s) {
    int length = s[0];
    char * b = z->b;
    int k = z->k;
    if (length > k + 1) return FALSE;
    if (memcmp(b, s + 1, length) != 0) return FALSE;
    z->j = length;
    return TRUE;
}

static void setto(struct stemmer * z, char * s) {
    int length = s[0];
    int j = z->j;
    memmove(z->b + j + 1, s + 1, length);
    z->k = j+length;
}

#define R1(z)    (z->j + 1 >= z->p1)
#define R2(z)    (z->j + 1 >= z->p2)

static void r(struct stemmer * z, char * s) { if (R1(z)) setto(z, s); }

static void step0(struct stemmer * z) {
    char * b = z->b;
    switch (b[z->k]) {
        case '\'':
            if (ends(z, "\03" "\'s\'")) z->k -= 3; else z->k -= 1;
            return;
        case 's':
            if (ends(z, "\02" "\'s")) z->k -= 2;
    }
}

static void step1a(struct stemmer * z) {
    char * b = z->b;
    if (b[z->k] == 'd') {
        if (ends(z, "\03" "ied")) b[z->k] = 's';
    }
    if (b[z->k] == 's') {
        if (ends(z, "\04" "sses")) z->k -= 2; else
        if (ends(z, "\03" "ies")) {
          /*
            if (z->j == 0)
                setto(z, "\02" "ie");
            else
                setto(z, "\01" "i");
          */
            if (z->j > 0)
                setto(z, "\01" "i");
            else
                setto(z, "\02" "ie");

        } else
        if (b[z->k - 1] != 's' && b[z->k - 1] != 'u') {
            int i;
            for (i = 0; i < z->k - 1; i++) if vowel(b[i]) {
                z->k--; break;
            }
        }
    }
}

static void step1b(struct stemmer * z) {
    switch (z->b[z->k]) {
        case 'd':
            if (ends(z, "\03" "eed")) { r(z, "\02" "ee"); break; }
            if (ends(z, "\02" "ed")) goto ed_test;
            break;
        case 'y':
            if (ends(z, "\05" "eedly")) { r(z, "\02" "ee"); break; }
            if (ends(z, "\04" "edly")) goto ed_test;
            if (ends(z, "\05" "ingly")) goto ed_test;
            break;
        case 'g':
            if (ends(z, "\03" "ing")) goto ed_test;
            break;
    }
    return;
ed_test:
    if (vowelinstem(z)) {
        setto(z, "\00");
        if (ends(z, "\02" "at")) setto(z, "\03" "ate"); else
        if (ends(z, "\02" "bl")) setto(z, "\03" "ble"); else
        if (ends(z, "\02" "iz")) setto(z, "\03" "ize"); else
        if (doublec(z, z->k)) {
            z->k--;
        }
        else if (z->j + 1 == z->p1 && shortv(z)) setto(z, "\01" "e");
    }
}

static void step1c(struct stemmer * z) {
    if (ends(z, "\01" "y") || ends(z, "\01" "Y"))
        if (z->k > 1 && cons(z->b[z->k - 1]))
            z->b[z->k] = 'i';
}

static void step2(struct stemmer * z) {
    switch (z->b[z->k - 1]) {
        case 'a':
            if (ends(z, "\07" "ational")) { r(z, "\03" "ate"); break; }
            if (ends(z, "\06" "tional")) { r(z, "\04" "tion"); break; }
            break;
        case 'c':
            if (ends(z, "\04" "enci")) { r(z, "\04" "ence"); break; }
            if (ends(z, "\04" "anci")) { r(z, "\04" "ance"); break; }
            break;
        case 'e':
            if (ends(z, "\04" "izer")) { r(z, "\03" "ize"); break; }
            break;
        case 'l':
            if (ends(z, "\03" "bli")) { r(z, "\03" "ble"); break; }
            if (ends(z, "\04" "alli")) { r(z, "\02" "al"); break; }
            if (ends(z, "\05" "entli")) { r(z, "\03" "ent"); break; }
            if (ends(z, "\03" "eli")) { r(z, "\01" "e"); break; }
            if (ends(z, "\05" "ousli")) { r(z, "\03" "ous"); break; }
            if (ends(z, "\05" "fulli")) { r(z, "\03" "ful"); break; }
            if (ends(z, "\06" "lessli")) { r(z, "\04" "less"); break; }
            if (ends(z, "\02" "li") && valid_li_ending(z)) {r(z, "\00"); break; }
            break;
        case 'o':
            if (ends(z, "\07" "ization")) { r(z, "\03" "ize"); break; }
            if (ends(z, "\05" "ation")) { r(z, "\03" "ate"); break; }
            if (ends(z, "\04" "ator")) { r(z, "\03" "ate"); break; }
            break;
        case 's':
            if (ends(z, "\05" "alism")) { r(z, "\02" "al"); break; }
            if (ends(z, "\07" "iveness")) { r(z, "\03" "ive"); break; }
            if (ends(z, "\07" "fulness")) { r(z, "\03" "ful"); break; }
            if (ends(z, "\07" "ousness")) { r(z, "\03" "ous"); break; }
            break;
        case 't':
            if (ends(z, "\05" "aliti")) { r(z, "\02" "al"); break; }
            if (ends(z, "\05" "iviti")) { r(z, "\03" "ive"); break; }
            if (ends(z, "\06" "biliti")) { r(z, "\03" "ble"); break; }
            break;
        case 'g':
            if (ends(z, "\03" "ogi") && z->b[z->j] == 'l') { r(z, "\02" "og"); break; }
     }
}

static void step3(struct stemmer * z) {
    switch (z->b[z->k]) {
        case 'e':
            if (ends(z, "\05" "icate")) { r(z, "\02" "ic"); break; }
            if (ends(z, "\05" "ative")) { if (R2(z)) setto(z, "\00"); break; }
            if (ends(z, "\05" "alize")) { r(z, "\02" "al"); break; }
            break;
        case 'i':
            if (ends(z, "\05" "iciti")) { r(z, "\02" "ic"); break; }
            break;
        case 'l':
            if (ends(z, "\04" "ical")) { r(z, "\02" "ic"); break; }
            if (ends(z, "\03" "ful")) { r(z, "\00"); break; }
            if (ends(z, "\07" "ational")) { r(z, "\03" "ate"); break; }
            if (ends(z, "\06" "tional")) { r(z, "\04" "tion"); break; }
            break;
        case 's':
            if (ends(z, "\04" "ness")) { r(z, "\00"); break; }
            break;
    }
}

static void step4(struct stemmer * z) {
    switch (z->b[z->k - 1]) {
        case 'a':
            if (ends(z, "\02" "al")) break; return;
        case 'c':
            if (ends(z, "\04" "ance")) break;
            if (ends(z, "\04" "ence")) break; return;
        case 'e':
            if (ends(z, "\02" "er")) break; return;
        case 'i':
            if (ends(z, "\02" "ic")) break; return;
        case 'l':
            if (ends(z, "\04" "able")) break;
            if (ends(z, "\04" "ible")) break; return;
        case 'n':
            if (ends(z, "\03" "ant")) break;
            if (ends(z, "\05" "ement")) break;
            if (ends(z, "\04" "ment")) break;
            if (ends(z, "\03" "ent")) break; return;
        case 'o':
            if (ends(z, "\03" "ion") && (z->b[z->j] == 's' || z->b[z->j] == 't')) break;
        case 's':
            if (ends(z, "\03" "ism")) break; return;
        case 't': if (ends(z, "\03" "ate")) break;
            if (ends(z, "\03" "iti")) break; return;
        case 'u':
            if (ends(z, "\03" "ous")) break; return;
        case 'v':
            if (ends(z, "\03" "ive")) break; return;
        case 'z':
            if (ends(z, "\03" "ize")) break; return;
      default:
            return;
   }
   if (R2(z)) z->k = z->j;
}

static void step5(struct stemmer * z) {
    if (ends(z, "\01" "e")) {
        if (R2(z) || R1(z) && !shortv(z)) setto(z, "\00");
        return;
    }
    if (ends(z, "\01" "l")) {
        if (R2(z) && z->b[z->j] == 'l') setto(z, "\00");
    }
}

static int exception1(struct stemmer * z) {
    switch(z->b[0]) {
        case 's':
            if (equals(z, "\05" "skies")) { setto(z, "\03" "sky"); return TRUE; }
            if (equals(z, "\04" "skis")) { setto(z, "\03" "ski"); return TRUE; }
            if (equals(z, "\03" "sky")) return TRUE;
            if (equals(z, "\06" "singly")) { setto(z, "\06" "singl"); return TRUE; }
            return FALSE;
        case 'd':
            if (equals(z, "\05" "dying")) { setto(z, "\03" "die"); return TRUE; }
            return FALSE;
        case 'l':
            if (equals(z, "\05" "lying")) { setto(z, "\03" "lie"); return TRUE; }
            return FALSE;
        case 't':
            if (equals(z, "\05" "tying")) { setto(z, "\03" "tie"); return TRUE; }
            return FALSE;
        case 'i':
            if (equals(z, "\04" "idly")) { setto(z, "\03" "idl"); return TRUE; }
            return FALSE;
        case 'g':
            if (equals(z, "\06" "gently")) { setto(z, "\05" "gentl"); return TRUE; }
            return FALSE;
        case 'u':
            if (equals(z, "\04" "ugly")) { setto(z, "\04" "ugli"); return TRUE; }
            return FALSE;
        case 'o':
            if (equals(z, "\04" "only")) { setto(z, "\04" "onli"); return TRUE; }
            return FALSE;
        case 'e':
            if (equals(z, "\05" "early")) { setto(z, "\05" "earli"); return TRUE; }
            return FALSE;
        case 'n':
            if (equals(z, "\04" "news")) return TRUE;
            return FALSE;
        case 'h':
            if (equals(z, "\04" "howe")) return TRUE;
            return FALSE;
        case 'a':
            if (equals(z, "\05" "atlas")) return TRUE;
            if (equals(z, "\05" "andes")) return TRUE;
            return FALSE;
        case 'c':
            if (equals(z, "\06" "cosmos")) return TRUE;
            return FALSE;
        case 'b':
            if (equals(z, "\04" "bias")) return TRUE;
            return FALSE;
    }
    return FALSE;
}

static int exception2(struct stemmer * z) {
    switch(z->b[0]) {
        case 'i':
            if (equals(z, "\06" "inning")) return TRUE;
            return FALSE;
        case 'o':
            if (equals(z, "\06" "outing")) return TRUE;
            return FALSE;
        case 'c':
            if (equals(z, "\07" "canning")) return TRUE;
            return FALSE;
        case 'h':
            if (equals(z, "\07" "herring")) return TRUE;
            return FALSE;
        case 'e':
            if (equals(z, "\07" "earring")) return TRUE;
            if (equals(z, "\06" "exceed")) return TRUE;
            return FALSE;
        case 'p':
            if (equals(z, "\07" "proceed")) return TRUE;
            return FALSE;
        case 's':
            if (equals(z, "\07" "succeed")) return TRUE;
            return FALSE;
    }
    return FALSE;
}

static int exception_p1(struct stemmer * z) {
    switch(z->b[0]) {
        case 'g':
            if (starts(z, "\05" "gener")) break;
            return FALSE;
        case 'c':
            if (starts(z, "\06" "commun")) break;
            return FALSE;
        default:
            return FALSE;
    }
    z->p1 = z->j;
    return TRUE;
}

/* In stem(z, b, k), b is a char pointer, and the string to be stemmed is
   from b[0] to b[k] inclusive.  Possibly b[k+1] == '\0', but it is not
   important. The stemmer adjusts the characters b[0] ... b[k] and returns
   the new end-point of the string, k'. Stemming never increases word
   length, so 0 <= k' <= k.
*/

extern int stem(struct stemmer * z, char * b, int k) {
    int i = 0;
    int j = 0;
    int Y_found = FALSE;
    if (k <= 1) return k;
    if (b[0] == '\'') i = 1;
    for (; i <= k; i++) {
        int ch = b[i];
        ch = tolower(ch);
        b[j++] = ch;
    }
    z->b = b;
    z->k = j - 1; /* copy the parameters into z */
    if (exception1(z)) return z->k;
    for (i = 0; i < j; i++) {
        int ch = b[i];
        if (ch == 'y') {
            if (i == 0 && j > 1 && vowel(b[1]) || i > 0 && vowel(b[i - 1])) {
                Y_found = TRUE; b[i] = ch = 'Y';
            }
        }
    }
    z->p1 = z->p2 = j;

    {   /* mark regions */
        int syll_count = 0;
        int ch_cons;
        int ch_prev_vowel = FALSE;
        i = 0;
        if (exception_p1(z)) {
            i = z->p1; syll_count = 1;
        }
        for (; i < j; i++) {
            int ch = b[i];
            ch_cons = !vowel(ch);
            if (ch_cons && ch_prev_vowel) {
                syll_count++;
                if (syll_count == 1)
                    z->p1 = i + 1;
                if (syll_count == 2) {
                    z->p2 = i + 1;
                    break;
                }
            }
            ch_prev_vowel = !ch_cons;
        }
    }

    step0(z); step1a(z);
    if (!exception2(z)) {
        step1b(z); step1c(z); step2(z); step3(z); step4(z); step5(z);
    }
    if (Y_found) for (i = 0; i <= k; i++) if (b[i] == 'Y') b[i] = 'y';
    return z->k;
}

/*--------------------stemmer definition ends here------------------------*/

#if 0

#include <stdio.h>
#include <stdlib.h>      /* for malloc, free */
#include <ctype.h>       /* for isupper, islower, tolower */

static char * s;         /* buffer for words tobe stemmed */

#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */

#define LETTER(ch) (isupper(ch) || islower(ch) || ch == '\'')

void stemfile(struct stemmer * z, FILE * f) {
    while(TRUE) {
        int ch = getc(f);
        if (ch == EOF) return;
        if (LETTER(ch)) {
            int i = 0;
            while(TRUE) {
                if (i == i_max) {
                    i_max += INC;
                    s = realloc(s, i_max + 1);
                }
                /* ch is made lower case in the call to stem(...) */
                s[i] = ch; i++;
                ch = getc(f);
                if (!LETTER(ch)) { ungetc(ch,f); break; }
            }
            s[stem(z, s, i - 1) + 1] = 0;
            /* the previous line calls the stemmer and uses its result to
                zero-terminate the string in s */
            printf("%s", s);
        }
        else putchar(ch);
    }
}

int main(int argc, char * argv[]) {
    int i;

    struct stemmer * z = create_stemmer();

    s = (char *) malloc(i_max + 1);
    for (i = 1; i < argc; i++) {
        FILE * f = fopen(argv[i],"r");
        if (f == 0) { fprintf(stderr,"File %s not found\n",argv[i]); exit(1); }
        stemfile(z, f);
    }
    free(s);

    free_stemmer(z);

    return 0;
}

#endif

