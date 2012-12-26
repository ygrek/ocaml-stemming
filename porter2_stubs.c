#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>

#include "porter2_c.h"

CAMLprim value caml_porter2_create(value v_unit)
{
  CAMLparam1(v_unit);
  struct stemmer* st = create_stemmer();
  CAMLreturn((value)st);
}

CAMLprim value caml_porter2_stem(value v_stem, value v_str)
{
  CAMLparam2(v_stem,v_str);
  CAMLlocal1(v_res);

  char* word = strdup(String_val(v_str));
  const int len = strlen(word);
  int i;

  if (NULL == word || 0 == len)
  {
    free(word);
    CAMLreturn(caml_copy_string(""));
  }
  if (len < 3) /* short words are not lowercased by stem() */
    for (i = 0; i < len; i++) word[i]=tolower(word[i]);
  int count = stem((struct stemmer*)v_stem, word, len - 1);

  /* consider 'singly' */
  word[count+1] = '\0';
  v_res = caml_copy_string(word);
  free(word);

  CAMLreturn(v_res);
}

CAMLprim value caml_porter2_finish(value v_stem)
{
  CAMLparam1(v_stem);
  free_stemmer((struct stemmer*)v_stem);
  CAMLreturn(Val_unit);
}

