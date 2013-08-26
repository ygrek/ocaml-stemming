
#include <string.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>

#include "keva_stemmers.h"
#include "fuzzyukr.h"
#include "fuzzyrus.h"

typedef struct stemmer_t
{
	char *buf;
	size_t len;
	stemScan ukr;
	stemScan rus;
} stemmer_t;

CAMLprim value caml_stemmer_keva_init(value v_unit)
{
	CAMLparam1(v_unit);

	stemmer_t *val = malloc(sizeof(stemmer_t));

	memset((void*)val, 0, sizeof(stemmer_t));
	val->buf = malloc(128);
	val->len = 128;
	
	val->ukr.lpdict = GetUkrTables();
	val->ukr.ptable = NULL;
	val->ukr.vowels = GetUkrVowels();
	val->ukr.minlen = 3;

	val->rus.lpdict = GetRusTables();
	val->rus.ptable = NULL;
	val->rus.vowels = GetRusVowels();
	val->rus.minlen = 3;

	CAMLreturn((value)val);
}

CAMLprim value caml_stemmer_keva_stem(value v_st, value v_str)
{
	CAMLparam2(v_st,v_str);
	CAMLlocal1(v_res);

	stemmer_t *st = (stemmer_t*)v_st;
	const char *istr = String_val(v_str);
	const size_t ilen = caml_string_length(v_str);

	if(ilen > st->len)
	{
		st->buf = realloc(st->buf, ilen + 1);
		st->len = ilen;
	}
  strcpy(st->buf, istr);

	int stems = 0;
	unsigned int buffer[3];
	unsigned int ulen, rlen;
	
	memset(buffer, 0, sizeof(buffer));
	stems = GetStemLenBuffer(&st->ukr, buffer, 3, st->buf, ilen);
	if(!stems) ulen = ilen; else ulen = buffer[0];
	
	memset(buffer, 0, sizeof(buffer));
	stems = GetStemLenBuffer(&st->rus, buffer, 3, st->buf, ilen);
	if(!stems) rlen = ilen; else rlen = buffer[0];
	
	const size_t olen = ulen < rlen ? ulen : rlen;

	v_res = caml_alloc_string(olen);
	memcpy(String_val(v_res),st->buf,olen);
	CAMLreturn(v_res);
}

CAMLprim value caml_stemmer_keva_close(value v_stemmer)
{
	CAMLparam1(v_stemmer);
	struct stemmer_t *val = (struct stemmer_t*)v_stemmer;
	free(val->buf);
	free(val);
	CAMLreturn(Val_unit);
}
