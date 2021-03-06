#include "standard.h"
#include "process_query.h"
#include "util.h"
#include "log.h"

#define MAX_QUERY_LENGTH 16384 /* maximum length of a query string */

geoname_idx_t geoname_idx(geoname_indices_t v, int idx) {
    return *(geoname_idx_t *) vector_at(v, idx);
}

/* Get lists of geoname indices for all of the given tokens and
   return no more than max_results entries from their intersection. */
static geoname_indices_t process_query(vector_t tokens, int max_results,
                                       geonames_by_token_func func) {
    int i, ntokens = vector_size(tokens);

    /* Lists of tokens. */
    geoname_indices_t   *lists = xmalloc(ntokens * sizeof(vector_t));
    /* Size of each list. */
    int                 *sizes = xmalloc(ntokens * sizeof(int));
    /* Current index in the intersection algorithm in each of the lists. */
    int                 *pos   = xmalloc(ntokens * sizeof(int));

    geoname_idx_t idx;
    geoname_indices_t res = vector_init(sizeof(geoname_idx_t));

    /* Get geoname indices for each token. */
    for (i = 0; i < ntokens; ++i) {
        char const *token = *(char const **) vector_at(tokens, i);

        lists[i] = func(token);
        sizes[i] = vector_size(lists[i]);
        pos[i]   = 0;

        if (!sizes[i])
            goto end;
    }

    idx = geoname_idx(lists[0], pos[0]);

    /* Find intersection of the results. */
    for (;;) {
        geoname_idx_t prev = idx;

        for (i = 0; i < ntokens; ++i) {
            while (pos[i] < sizes[i] &&
                   geoname_idx(lists[i], pos[i]) < idx)
                ++pos[i];
            if (pos[i] == sizes[i])
                goto end;
            idx = geoname_idx(lists[i], pos[i]);
        }

        if (prev == idx) {
            vector_push(res, &idx);
            if (max_results == vector_size(res))
                goto end;
            ++idx;
        }
    }

end:
    free(pos);
    free(sizes);
    free(lists);

    return res;
}

void run_interactive_loop(geonames_by_token_func geonames_func,
                          int max_results,
                          process_geoname_id_func process_func) {
    char q[MAX_QUERY_LENGTH];

    debug("Ready to serve\n");

    for (;;) {
        int i;
        vector_t tokens;
        geoname_indices_t geonames;

        if (!fgets(q, sizeof q, stdin))
            break;

        strlower(strtrim(q));

        if (!*q) {
            puts("");
            continue;
        }

        tokens = strsplit(q, " \t");
        geonames = process_query(tokens, max_results, geonames_func);

        for (i = 0; i != vector_size(geonames); ++i)
            process_func(geoname_idx(geonames, i));

        puts("");

        vector_free(tokens);
        vector_free(geonames);
    }
}
