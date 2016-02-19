#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dns_message.h"
#include "md_array.h"
#include "pcap.h"
#include "base64.h"
#include "xmalloc.h"

static const char *d1_type_s;	/* XXX barf */
static const char *d2_type_s;	/* XXX barf */

// TODO
//static const char *b64 = ", { \"base64\": 1}";

static char comma[3] = {0};  /* Comma-separators manager  */

static void
start_array(void *pr_data, const char *name)
{
    FILE *fp = pr_data;
    assert(fp);

    if (comma[0]){
        fprintf(fp, ",\n");
    } else {
        comma[0]=1;
    }
    fprintf(fp, "  ");

    fprintf(fp, "\"%s\": {\n", name);
    fprintf(fp, "    \"start_time\": { \"$date\": %d000 },\n", Pcap_start_time());
    fprintf(fp, "    \"stop_time\": { \"$date\": %d000 },\n", Pcap_finish_time());
    fprintf(fp, "    \"dimensions\": [");
}

static void
finish_array(void *pr_data)
{
    FILE *fp = pr_data;
    comma[1] = 0;
    fprintf(fp, "  }");
}

static void
d1_type(void *pr_data, const char *t)
{
    FILE *fp = pr_data;
    fprintf(fp, " \"%s\"", t);
    d1_type_s = t;
}

static void
d2_type(void *pr_data, const char *t)
{
    FILE *fp = pr_data;
    fprintf(fp, ", \"%s\" ],\n", t);
    d2_type_s = t;
}

static const char *entity_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
"0123456789._-:";

static void
d1_begin(void *pr_data, char *l)
{
    FILE *fp = pr_data;
    int ll = strlen(l);
    char *e = NULL;
    if (strspn(l, entity_chars) != ll) {
        int x = base64_encode(l, ll, &e);
        assert(x);
        l = e;
    }

    if (comma[1]){
        fprintf(fp, ",\n");
    } else {
        comma[1]=1;
    }

    fprintf(fp, "      {\n");
    fprintf(fp, "        ");
    fprintf(fp, "\"%s\": \"%s\",\n", d1_type_s, l);
    fprintf(fp, "        ");
    fprintf(fp, "\"%s\": [", d2_type_s);

    if (e)
        xfree(e);
}

static void
print_element(void *pr_data, char *l, int val)
{
    FILE *fp = pr_data;
    int ll = strlen(l);
    char *e = NULL;
    if (strspn(l, entity_chars) != ll) {
        int x = base64_encode(l, ll, &e);
        assert(x);
        l = e;
    }

    if (comma[2]){
        fprintf(fp, ",");
    } else {
        comma[2]=1;
    }
    fprintf(fp, "\n          ");
    fprintf(fp, "{ \"val\": \"%s\", \"count\": %d }", l, val);

    if (e)
        xfree(e);
}

static void
d1_end(void *pr_data, char *l)
{
    FILE *fp = pr_data;

    if (comma[2]){
        fprintf(fp, "\n        ]");
    } else {
        fprintf(fp, " ]");
    }

    fprintf(fp, "\n      }");
    comma[2] = 0;
}

static void
start_data(void *pr_data)
{
    FILE *fp = pr_data;
    fprintf(fp, "    \"data\": [\n");
}

static void
finish_data(void *pr_data)
{
    FILE *fp = pr_data;
    fprintf(fp, "\n    ]\n");
}

md_array_printer json_printer =
{
    start_array,
    finish_array,
    d1_type,
    d2_type,
    start_data,
    finish_data,
    d1_begin,
    d1_end,
    print_element
};
