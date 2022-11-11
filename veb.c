#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef struct veb_node veb;
struct veb_node
{
    unsigned int u;
    unsigned int min;
    unsigned int max;
    veb *summary;
    veb **cluster;
};

const unsigned int NIL = -1;

unsigned int veb_upper_sqrt(unsigned int u){
    return pow(2, ceil(log2(u)/2));
}

unsigned int veb_log2(unsigned int u) {
    unsigned int result = 0;
    while (u >>= 1) result++;
    return result;
}

unsigned int veb_lower_sqrt(unsigned int u){
    // return pow(2, floor(log2(u)/2));
    // return 1U<<((int)log2(u) >> 1);
    return 1U<<(veb_log2(u) >> 1);
}


unsigned int veb_high(unsigned int u,  unsigned int x) {
    return floor(x/veb_lower_sqrt(u));
}

unsigned int veb_low(unsigned int u, unsigned int x) {
    return x % veb_lower_sqrt(u);
}

unsigned int veb_index(unsigned int u, unsigned int x, unsigned int y){
    return x*veb_lower_sqrt(u)+y;
}

unsigned int veb_tree_minimum(veb* v) {
    return v->min;
}

unsigned int veb_tree_maximum(veb* v) {
    return v->max;
}

unsigned int veb_tree_member(veb* v, unsigned int x){
    return
        x == v->min || x == v->max ? 1 :
        v->u == 2 ? 0 :
        veb_tree_member(v->cluster[veb_high(v->u,x)], veb_low(v->u, x));
}

unsigned int veb_tree_successor(veb* v, unsigned int x){
    if (v->u == 2)
    {
        if (x == 0 && v->max == 1) {
            return 1;
        } else {
            return NIL;
        }
    } else if (v->min != NIL && x < v->min)
    {
        return v->min;
    } else {
        unsigned int max_low = veb_tree_maximum(v->cluster[veb_high(v->u,x)]);
        if (max_low != NIL && veb_low(v->u, x) < max_low) {
            unsigned int offset = veb_tree_successor(v->cluster[veb_high(v->u, x)], veb_low(v->u,x));
            return veb_index(v->u, veb_high(v->u, x), offset);
        }
        else {
            unsigned int succ_cluster = veb_tree_successor(v->summary, veb_high(v->u,x));
            if (succ_cluster == NIL)
            {
                return NIL;
            } else {
                unsigned int offset = veb_tree_minimum(v->cluster[succ_cluster]);
                return veb_index(v->u, succ_cluster, offset);
            }
        }
    }
}

unsigned int veb_tree_predecessor(veb* v, unsigned int x){
    if (v->u == 2) {
        if (x == 1 && v->min == 0) {
            return 0;
        } else {
            return NIL;
        }
    } else if (v->max != NIL && x > v->max)
    {
        return v->max;
    } else {
        unsigned int min_low = veb_tree_minimum(v->cluster[veb_high(v->u, x)]);
        if (min_low != NIL && veb_low(v->u, x) > min_low) {
            unsigned int offset = veb_tree_predecessor(v->cluster[veb_high(v->u, x)], veb_low(v->u, x));
            return veb_index(v->u, veb_high(v->u, x), offset);
        } else {
            unsigned int pred_cluster = veb_tree_predecessor(v->summary, veb_high(v->u, x));
            if (pred_cluster == NIL) {
                if (v->min != NIL && x > v->min){
                    return v->min;
                } else {
                    return NIL;
                }
            } else {
                unsigned int offset = veb_tree_maximum(v->cluster[pred_cluster]);
                return veb_index(v->u, pred_cluster, offset);
            }
        }
    }
}

void veb_empty_tree_insert(veb* v, unsigned int x) {
    v->min = x;
    v->max = x;
}

void veb_exchange(unsigned int* a, unsigned int* b) {
   unsigned int temp = *a;
   *a = *b;
   *b = temp;
}

void veb_tree_insert(veb* v, unsigned int x) {
    if (v->min == NIL) {
        veb_empty_tree_insert(v, x);
    } else {
        if (x < v->min) { 
            veb_exchange(&x,&(v->min));
        }
        if (v->u > 2) {
            if (veb_tree_minimum(v->cluster[veb_high(v->u,x)]) == NIL) {
                veb_tree_insert(v->summary, veb_high(v->u, x));
                veb_empty_tree_insert(v->cluster[veb_high(v->u, x)], veb_low(v->u, x));
            } else {
                veb_tree_insert(v->cluster[veb_high(v->u, x)], veb_low(v->u, x));
            }
        }
        if (x > v->max)
        {
            v->max = x;
        }
        
    }
}

veb* create_veb(unsigned int u) {
    veb* new_veb = (veb*)malloc(sizeof(veb));
    
    if (!new_veb) return NULL;
    new_veb->max = NIL;
    new_veb->min = NIL;
    new_veb->u = u;
    if (u > 2) {
        unsigned int upper_sqrt = veb_upper_sqrt(u);
        new_veb->summary = create_veb(upper_sqrt);
        if (!(new_veb->summary)) return NULL;
        new_veb->cluster = (veb**)malloc(upper_sqrt * sizeof(veb));
        if (!(new_veb->cluster)) return NULL;
        for (int i = 0; i < upper_sqrt; i++){
            new_veb->cluster[i] = create_veb(upper_sqrt);
            if (!(new_veb->cluster[i])) return NULL;
        }
    }
    return new_veb;
}

void destroy_veb(veb* veb) {
    if (veb->u > 2) {
        unsigned int upper_sqrt = veb_upper_sqrt(veb->u);
        destroy_veb(veb->summary);
        for (int i = 0; i < upper_sqrt; i++) {
            destroy_veb(veb->cluster[i]);
        }
        free(veb->cluster);
    }
    free(veb);
}

void print_veb_contents(veb* veb) {
    unsigned int min = veb->min;
    while (min != NIL)
    {
        printf("%d ", min);
        min = veb_tree_successor(veb, min);
    }
}

void veb_tree_delete(veb* v, unsigned int x) {
    if (v->min == v->max)
    {
        v->max = NIL;
        v->min = NIL;
    } else if (v->u == 2)
    {
        if (x == 0)
        {
            v->min = 1;
        } else
        {
            v->min = 0;
            v->max = 0;
        } 
    } else if (x == v->min)
    {
        unsigned int first_cluster = veb_tree_minimum(v->summary);
        x = veb_index(v->u ,first_cluster, veb_tree_minimum(v->cluster[first_cluster]));
        v->min = x;
        veb_tree_delete(v->cluster[veb_high(v->u,x)], veb_low(v->u, x));
        if (veb_tree_minimum(v->cluster[veb_high(v->u, x)]) == NIL) {
            veb_tree_delete(v->summary, veb_high(v->u, x));
            if (x == v->max) {
                unsigned int summary_max = veb_tree_maximum(v->summary);
                if (summary_max == NIL)
                {
                    v->max = v->min;
                } else
                {
                    v->max = veb_index(v->u, summary_max, veb_tree_maximum(v->cluster[summary_max]));
                }       
            }
        } else if (x == v->max)
        {
            v->max = veb_index(v->u,veb_high(v->u, x), veb_tree_maximum(v->cluster[veb_high(v->u, x)]));
        }
    }
}