#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>

#include "porter2_c.h"

typedef struct stemmer_t {
  struct stemmer *st;
  char *buf;
  size_t len;
} stemmer_t;

CAMLprim value caml_stemmer_porter2_init(value v_unit)
{
  CAMLparam1(v_unit);
  stemmer_t *val = malloc(sizeof(stemmer_t));
  memset((void*)val, 0, sizeof(stemmer_t));
  val->st = create_stemmer();
  val->buf = malloc(32);
  val->len = 32;
  memset((void*)val->buf,0,32);
  CAMLreturn((value)val);
}

CAMLprim value caml_stemmer_porter2_stem(value v_stem, value v_str)
{
  CAMLparam2(v_stem,v_str);
  CAMLlocal1(v_res);

  stemmer_t* val = (stemmer_t*)v_stem;
  size_t i, len = caml_string_length(v_str);
  if(len >= val->len){
    val->len = len + 1; // to put trailing zero securely
    val->buf = realloc(val->buf, val->len);
  }
  
  len = 0;
  char *word = val->buf, *d = val->buf, *s = String_val(v_str);
  // This is much more beautiful and optimistic, but generates ugly
  // warnings. Sad.
  //while(*d++ = *s++) i++ ;
  while(*s && (len < val->len)){ *d = *s; d++; s++; len++; }
  if(0 == len) CAMLreturn(caml_copy_string(""));

  // short words are not lowercased by stem()
  if (len < 3){
    word[0] = tolower(word[0]);
    word[1] = tolower(word[1]);
  }

  i = stem(val->st, word, len-1);

  word[i+1] = '\0';
  v_res = caml_copy_string(word);

  CAMLreturn(v_res);
}

CAMLprim value caml_stemmer_porter2_close(value v_stem)
{
  CAMLparam1(v_stem);
  struct stemmer_t* val = (struct stemmer_t*)v_stem;
  free_stemmer(val->st);
  free(val->buf);
  free(val);
  CAMLreturn(Val_unit);
}
