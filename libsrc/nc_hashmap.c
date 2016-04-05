#include "nc_hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* this should be prime */
#define TABLE_STARTSIZE 1021

#define ACTIVE 1

/* NOTE: 'data' is the dimid or varid which is non-negative.
   we store the dimid+1 so a valid entry will have
   data > 0
*/
typedef struct {
  int data;
  int flags;
  long key;
} hEntry;

struct s_hashmap{
  hEntry* table;
  long size, count;
};

static unsigned long isPrime(unsigned long val)
{
  int i, p, exp, a;

  for (i = 9; i--;)
  {
    a = (rand() % (val-4)) + 2;
    p = 1;
    exp = val-1;
    while (exp)
    {
      if (exp & 1)
	p = (p*a)%val;

      a = (a*a)%val;
      exp >>= 1;
    }

    if (p != 1)
      return 0;
  }

  return 1;
}

static int findPrimeGreaterThan(int val)
{
  if (val & 1)
    val+=2;
  else
    val++;

  while (!isPrime(val))
    val+=2;

  return val;
}

static void rehash(NC_hashmap* hm)
{
  long size = hm->size;
  long count = hm->count;

  hEntry* table = hm->table;

  hm->size = findPrimeGreaterThan(size<<1);
  hm->table = (hEntry*)calloc(sizeof(hEntry), hm->size);
  hm->count = 0;

  while(--size >= 0) {
    if (table[size].flags == ACTIVE) {
      NC_hashmapInsert(hm, table[size].data-1, table[size].key);
      assert(NC_hashmapGet(hm, table[size].key) == table[size].data-1);
    }
  }

  free(table);
  assert(count == hm->count);
}

NC_hashmap* NC_hashmapCreate(int startsize)
{
  NC_hashmap* hm = (NC_hashmap*)malloc(sizeof(NC_hashmap));

  if (!startsize)
    startsize = TABLE_STARTSIZE;
  else {
    startsize *= 4;
    startsize /= 3;
    startsize = findPrimeGreaterThan(startsize-2);
  }

  hm->table = (hEntry*)calloc(sizeof(hEntry), startsize);
  hm->size = startsize;
  hm->count = 0;

  return hm;
}

void NC_hashmapInsert(NC_hashmap* hash, int data, unsigned long key)
{
  long index, i, step;

  if (hash->size*3/4 <= hash->count) {
    rehash(hash);
  }

  do
  {
    index = key % hash->size;
    step = (key % (hash->size-2)) + 1;

    for (i = 0; i < hash->size; i++)
    {
      if (hash->table[index].flags & ACTIVE)
      {
	if (hash->table[index].key == key)
	{
	  hash->table[index].data = data+1;
	  return;
	}
      }
      else
      {
	hash->table[index].flags |= ACTIVE;
	hash->table[index].data = data+1;
	hash->table[index].key = key;
	++hash->count;
	return;
      }

      index = (index + step) % hash->size;
    }

    /* it should not be possible that we EVER come this far, but unfortunately
       not every generated prime number is prime (Carmichael numbers...) */
    rehash(hash);
  }
  while (1);
}

int NC_hashmapRemove(NC_hashmap* hash, unsigned long key)
{
  long index, i, step;

  index = key % hash->size;
  step = (key % (hash->size-2)) + 1;

  for (i = 0; i < hash->size; i++)
  {
    if (hash->table[index].data > 0)
    {
      if (hash->table[index].key == key)
      {
	if (hash->table[index].flags & ACTIVE)
	{
	  hash->table[index].flags &= ~ACTIVE;
	  --hash->count;
	  return hash->table[index].data-1;
	}
	else /* in, but not active (i.e. deleted) */
	  return -1;
      }
    }
    else /* found an empty place (can't be in) */
      return -1;

    index = (index + step) % hash->size;
  }
  /* everything searched through, but not in */
  return -1;
}

int NC_hashmapGet(NC_hashmap* hash, unsigned long key)
{
  if (hash->count)
  {
    long index, i, step;
    index = key % hash->size;
    step = (key % (hash->size-2)) + 1;

    for (i = 0; i < hash->size; i++)
    {
      if (hash->table[index].key == key)
      {
	if (hash->table[index].flags & ACTIVE)
	  return hash->table[index].data-1;
	break;
      }
      else
	if (!(hash->table[index].flags & ACTIVE))
	  break;

      index = (index + step) % hash->size;
    }
  }

  return -1;
}

long NC_hashmapCount(NC_hashmap* hash)
{
  return hash->count;
}

void NC_hashmapDelete(NC_hashmap* hash)
{
  if (hash) {
    free(hash->table);
    free(hash);
  }
}
