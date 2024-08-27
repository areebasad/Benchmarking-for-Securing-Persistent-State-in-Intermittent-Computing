#include <atmel_start.h>

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#include <stdio.h>
#endif // DEBUG_PRINT

#include <stdlib.h>
#include <limits.h>

typedef uint16_t dtype;

typedef struct {
	dtype vertex;
	dtype weight;
} edge_t;

typedef struct {
	edge_t **edges;
	dtype edges_len;
	dtype edges_size;
	int dist;
	dtype prev;
	dtype visited;
} vertex_t;

typedef struct {
	vertex_t **vertices;
	dtype vertices_len;
	dtype vertices_size;
} graph_t;

typedef struct {
	dtype *data;
	dtype *prio;
	dtype *index;
	dtype len;
	dtype size;
} heap_t;

void add_vertex (graph_t *g, dtype i) {
	if (g->vertices_size < i + 1) {
		dtype size = g->vertices_size * 2 > i ? g->vertices_size * 2 : i + 4;
		g->vertices = realloc(g->vertices, size * sizeof (vertex_t *));
		for (dtype j = g->vertices_size; j < size; j++)
		g->vertices[j] = NULL;
		g->vertices_size = size;
	}
	if (!g->vertices[i]) {
		g->vertices[i] = calloc(1, sizeof (vertex_t));
		g->vertices_len++;
	}
}

void add_edge (graph_t *g, dtype a, dtype b, dtype w) {
	add_vertex(g, a);
	add_vertex(g, b);
	vertex_t *v = g->vertices[a];
	if (v->edges_len >= v->edges_size) {
		v->edges_size = v->edges_size ? v->edges_size * 2 : 4;
		v->edges = realloc(v->edges, v->edges_size * sizeof (edge_t *));
	}
	edge_t *e = calloc(1, sizeof (edge_t));
	e->vertex = b;
	e->weight = w;
	v->edges[v->edges_len++] = e;
}

heap_t *create_heap (dtype n) {
	heap_t *h = calloc(1, sizeof (heap_t));
	h->data = calloc(n + 1, sizeof (dtype));
	h->prio = calloc(n + 1, sizeof (dtype));
	h->index = calloc(n, sizeof (dtype));
	return h;
}

void push_heap (heap_t *h, dtype v, dtype p) {
	dtype i = h->index[v] == 0 ? ++h->len : h->index[v];
	dtype j = i / 2;
	while (i > 1) {
		if (h->prio[j] < p)
		break;
		h->data[i] = h->data[j];
		h->prio[i] = h->prio[j];
		h->index[h->data[i]] = i;
		i = j;
		j = j / 2;
	}
	h->data[i] = v;
	h->prio[i] = p;
	h->index[v] = i;
}

dtype min (heap_t *h, dtype i, dtype j, dtype k) {
	dtype m = i;
	if (j <= h->len && h->prio[j] < h->prio[m])
	m = j;
	if (k <= h->len && h->prio[k] < h->prio[m])
	m = k;
	return m;
}

dtype pop_heap (heap_t *h) {
	dtype v = h->data[1];
	dtype i = 1;
	while (1) {
		dtype j = min(h, h->len, 2 * i, 2 * i + 1);
		if (j == h->len)
		break;
		h->data[i] = h->data[j];
		h->prio[i] = h->prio[j];
		h->index[h->data[i]] = i;
		i = j;
	}
	h->data[i] = h->data[h->len];
	h->prio[i] = h->prio[h->len];
	h->index[h->data[i]] = i;
	h->len--;
	return v;
}

void dijkstra (graph_t *g, dtype a, dtype b) {
	dtype i, j;
	for (i = 0; i < g->vertices_len; i++) {
		vertex_t *v = g->vertices[i];
		v->dist = INT_MAX;
		v->prev = 0;
		v->visited = 0;
	}
	vertex_t *v = g->vertices[a];
	v->dist = 0;
	heap_t *h = create_heap(g->vertices_len);
	push_heap(h, a, v->dist);
	while (h->len) {
		i = pop_heap(h);
		if (i == b)
		break;
		v = g->vertices[i];
		v->visited = 1;
		for (j = 0; j < v->edges_len; j++) {
			edge_t *e = v->edges[j];
			vertex_t *u = g->vertices[e->vertex];
			if (!u->visited && v->dist + e->weight <= u->dist) {
				u->prev = i;
				u->dist = v->dist + e->weight;
				push_heap(h, e->vertex, u->dist);
			}
		}
	}
}

#ifdef DEBUG_PRINT
void print_path (graph_t *g, dtype i) {
	dtype n;
	vertex_t *v, *u;
	v = g->vertices[i];
	if (v->dist == INT_MAX) {
		printf("no path\n");
		return;
	}
	for (n = 1, u = v; u->dist; u = g->vertices[u->prev], n++)
	;
	printf("length: %d hops: %d, path: %d", v->dist, n-1, i);
	for (u = v; u->dist; u = g->vertices[u->prev])
	printf("<-%d",u->prev);
}
#endif // DEBUG_PRINT


int main (void) {
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	START_MEASURE(DGI_GPIO3);
	graph_t *g = calloc(1, sizeof (graph_t));
	add_edge(g, 25, 69, 197);
	add_edge(g, 56, 50, 197);
	add_edge(g, 15, 107, 36);
	add_edge(g, 26, 79, 103);
	add_edge(g, 39, 104, 103);
	add_edge(g, 4, 92, 224);
	add_edge(g, 10, 49, 17);
	add_edge(g, 19, 73, 49);
	add_edge(g, 82, 15, 39);
	add_edge(g, 9, 115, 239);
	add_edge(g, 113, 31, 157);
	add_edge(g, 48, 44, 22);
	add_edge(g, 15, 118, 125);
	add_edge(g, 67, 5, 248);
	add_edge(g, 73, 53, 189);
	add_edge(g, 97, 43, 59);
	add_edge(g, 0, 124, 86);
	add_edge(g, 72, 84, 0);
	add_edge(g, 5, 59, 125);
	add_edge(g, 107, 13, 12);
	add_edge(g, 16, 8, 37);
	add_edge(g, 25, 32, 153);
	add_edge(g, 73, 66, 250);
	add_edge(g, 67, 30, 244);
	add_edge(g, 18, 108, 54);
	add_edge(g, 72, 38, 146);
	add_edge(g, 109, 8, 32);
	add_edge(g, 74, 69, 227);
	add_edge(g, 58, 86, 108);
	add_edge(g, 116, 29, 118);
	add_edge(g, 115, 57, 75);
	add_edge(g, 99, 32, 142);
	add_edge(g, 84, 127, 91);
	add_edge(g, 49, 73, 128);
	add_edge(g, 78, 70, 235);
	add_edge(g, 125, 2, 12);
	add_edge(g, 82, 59, 61);
	add_edge(g, 90, 20, 195);
	add_edge(g, 98, 14, 104);
	add_edge(g, 34, 57, 119);
	add_edge(g, 6, 126, 72);
	add_edge(g, 18, 104, 56);
	add_edge(g, 70, 11, 76);
	add_edge(g, 18, 27, 227);
	add_edge(g, 80, 55, 107);
	add_edge(g, 86, 90, 25);
	add_edge(g, 67, 101, 254);
	add_edge(g, 42, 44, 195);
	add_edge(g, 84, 87, 54);
	add_edge(g, 49, 96, 54);
	add_edge(g, 103, 104, 69);
	add_edge(g, 124, 87, 205);
	add_edge(g, 106, 53, 135);
	add_edge(g, 118, 21, 138);
	add_edge(g, 58, 109, 78);
	add_edge(g, 36, 93, 110);
	add_edge(g, 10, 44, 125);
	add_edge(g, 23, 16, 78);
	add_edge(g, 11, 57, 58);
	add_edge(g, 107, 95, 185);
	add_edge(g, 122, 37, 155);
	add_edge(g, 31, 83, 148);
	add_edge(g, 119, 17, 39);
	add_edge(g, 127, 90, 51);
	add_edge(g, 108, 107, 61);
	add_edge(g, 43, 79, 55);
	add_edge(g, 125, 6, 114);
	add_edge(g, 55, 117, 93);
	add_edge(g, 107, 26, 254);
	add_edge(g, 23, 5, 169);
	add_edge(g, 62, 28, 155);
	add_edge(g, 45, 125, 238);
	add_edge(g, 122, 53, 85);
	add_edge(g, 90, 37, 92);
	add_edge(g, 53, 3, 150);
	add_edge(g, 30, 47, 127);
	add_edge(g, 100, 102, 182);
	add_edge(g, 51, 0, 77);
	add_edge(g, 96, 94, 97);
	add_edge(g, 126, 102, 156);
	add_edge(g, 87, 65, 61);
	add_edge(g, 77, 5, 57);
	add_edge(g, 83, 98, 86);
	add_edge(g, 31, 5, 205);
	add_edge(g, 76, 110, 186);
	add_edge(g, 27, 46, 56);
	add_edge(g, 65, 87, 130);
	add_edge(g, 77, 12, 44);
	add_edge(g, 45, 98, 250);
	add_edge(g, 42, 22, 196);
	add_edge(g, 82, 117, 117);
	add_edge(g, 84, 42, 68);
	add_edge(g, 98, 36, 238);
	add_edge(g, 109, 47, 206);
	add_edge(g, 28, 24, 34);
	add_edge(g, 8, 105, 172);
	add_edge(g, 64, 51, 117);
	add_edge(g, 23, 117, 12);
	add_edge(g, 96, 120, 228);
	add_edge(g, 111, 67, 206);
	add_edge(g, 92, 4, 51);
	add_edge(g, 47, 64, 104);
	add_edge(g, 121, 29, 189);
	add_edge(g, 12, 49, 239);
	add_edge(g, 32, 82, 187);
	add_edge(g, 125, 69, 69);
	add_edge(g, 68, 121, 170);
	add_edge(g, 49, 82, 121);
	add_edge(g, 47, 71, 149);
	add_edge(g, 76, 105, 79);
	add_edge(g, 113, 101, 135);
	add_edge(g, 87, 29, 7);
	add_edge(g, 106, 43, 122);
	add_edge(g, 107, 115, 186);
	add_edge(g, 123, 2, 207);
	add_edge(g, 45, 82, 82);
	add_edge(g, 35, 120, 175);
	add_edge(g, 6, 43, 39);
	add_edge(g, 108, 20, 86);
	add_edge(g, 108, 1, 255);
	add_edge(g, 60, 119, 91);
	add_edge(g, 117, 16, 188);
	add_edge(g, 63, 31, 83);
	add_edge(g, 32, 81, 218);
	add_edge(g, 79, 30, 61);
	add_edge(g, 68, 88, 215);
	add_edge(g, 112, 74, 207);
	add_edge(g, 8, 82, 74);
	add_edge(g, 77, 63, 248);
	add_edge(g, 7, 35, 4);
	add_edge(g, 59, 18, 69);
	add_edge(g, 111, 123, 10);
	add_edge(g, 43, 40, 57);
	add_edge(g, 97, 87, 135);
	add_edge(g, 77, 121, 194);
	add_edge(g, 51, 90, 168);
	add_edge(g, 57, 93, 145);
	add_edge(g, 51, 70, 178);
	add_edge(g, 96, 2, 201);
	add_edge(g, 24, 34, 92);
	add_edge(g, 35, 87, 46);
	add_edge(g, 48, 70, 204);
	add_edge(g, 4, 27, 13);
	add_edge(g, 81, 73, 47);
	add_edge(g, 44, 62, 22);
	add_edge(g, 107, 119, 163);
	add_edge(g, 59, 81, 140);
	add_edge(g, 56, 21, 215);
	add_edge(g, 4, 95, 191);
	add_edge(g, 65, 115, 82);
	add_edge(g, 53, 76, 144);
	add_edge(g, 21, 76, 53);
	add_edge(g, 78, 8, 196);
	add_edge(g, 119, 121, 63);
	add_edge(g, 80, 17, 194);
	add_edge(g, 54, 21, 137);
	add_edge(g, 17, 80, 105);
	add_edge(g, 50, 10, 55);
	add_edge(g, 29, 10, 189);
	add_edge(g, 80, 107, 90);
	add_edge(g, 70, 54, 98);
	add_edge(g, 39, 27, 84);
	add_edge(g, 42, 39, 14);
	add_edge(g, 104, 79, 179);
	add_edge(g, 29, 90, 237);
	add_edge(g, 70, 25, 249);
	add_edge(g, 38, 5, 95);
	add_edge(g, 11, 115, 207);
	add_edge(g, 72, 78, 203);
	add_edge(g, 91, 66, 208);
	add_edge(g, 121, 0, 46);
	add_edge(g, 67, 24, 105);
	add_edge(g, 84, 119, 55);
	add_edge(g, 50, 51, 127);
	add_edge(g, 78, 101, 118);
	add_edge(g, 47, 79, 73);
	add_edge(g, 105, 91, 180);
	add_edge(g, 78, 125, 171);
	add_edge(g, 70, 107, 69);
	add_edge(g, 106, 126, 199);
	add_edge(g, 126, 84, 109);
	add_edge(g, 27, 48, 96);
	add_edge(g, 125, 34, 218);
	add_edge(g, 76, 3, 90);
	add_edge(g, 109, 97, 106);
	add_edge(g, 127, 38, 12);
	add_edge(g, 68, 43, 193);
	add_edge(g, 41, 46, 11);
	add_edge(g, 62, 60, 12);
	add_edge(g, 5, 74, 186);
	add_edge(g, 12, 125, 164);
	add_edge(g, 35, 90, 24);
	add_edge(g, 35, 10, 181);
	add_edge(g, 99, 18, 199);
	add_edge(g, 63, 32, 177);
	add_edge(g, 118, 75, 255);
	add_edge(g, 43, 93, 96);
	add_edge(g, 52, 117, 111);
	add_edge(g, 54, 9, 153);
	add_edge(g, 96, 1, 238);
	add_edge(g, 42, 63, 177);
	add_edge(g, 79, 61, 214);
	add_edge(g, 59, 53, 197);
	add_edge(g, 0, 116, 24);
	add_edge(g, 25, 76, 210);
	add_edge(g, 85, 62, 185);
	add_edge(g, 107, 124, 96);
	add_edge(g, 96, 116, 17);
	add_edge(g, 114, 109, 46);
	add_edge(g, 116, 23, 23);
	add_edge(g, 77, 95, 254);
	add_edge(g, 81, 113, 184);
	add_edge(g, 73, 26, 71);
	add_edge(g, 54, 68, 103);
	add_edge(g, 79, 10, 143);
	add_edge(g, 4, 114, 64);
	add_edge(g, 3, 83, 5);
	add_edge(g, 108, 26, 137);
	add_edge(g, 71, 62, 45);
	add_edge(g, 43, 11, 240);
	add_edge(g, 90, 69, 80);
	add_edge(g, 47, 123, 152);
	add_edge(g, 72, 18, 143);
	add_edge(g, 81, 103, 181);
	add_edge(g, 1, 54, 118);
	add_edge(g, 95, 94, 35);
	add_edge(g, 37, 89, 171);
	add_edge(g, 90, 76, 4);
	add_edge(g, 34, 79, 115);
	add_edge(g, 37, 58, 57);
	add_edge(g, 75, 108, 80);
	add_edge(g, 42, 109, 252);
	add_edge(g, 9, 100, 163);
	add_edge(g, 92, 94, 128);
	add_edge(g, 70, 82, 15);
	add_edge(g, 18, 12, 11);
	add_edge(g, 42, 98, 73);
	add_edge(g, 63, 117, 104);
	add_edge(g, 108, 66, 170);
	add_edge(g, 103, 90, 243);
	add_edge(g, 118, 5, 234);
	add_edge(g, 90, 17, 238);
	add_edge(g, 15, 100, 172);
	add_edge(g, 64, 63, 163);
	add_edge(g, 104, 49, 37);
	add_edge(g, 94, 80, 207);
	add_edge(g, 68, 36, 107);
	add_edge(g, 12, 75, 251);
	add_edge(g, 5, 4, 172);
	add_edge(g, 56, 48, 134);
	add_edge(g, 28, 89, 213);
	add_edge(g, 104, 54, 134);
	add_edge(g, 10, 70, 247);
	add_edge(g, 69, 48, 80);
	add_edge(g, 69, 98, 222);
	add_edge(g, 19, 43, 153);
	add_edge(g, 42, 6, 227);
	add_edge(g, 107, 32, 159);
	add_edge(g, 52, 106, 247);
	add_edge(g, 98, 43, 154);
	add_edge(g, 27, 77, 191);
	add_edge(g, 67, 27, 105);
	add_edge(g, 31, 51, 238);
	add_edge(g, 110, 29, 67);
	add_edge(g, 106, 89, 245);
	add_edge(g, 19, 38, 197);
	add_edge(g, 112, 69, 184);
	add_edge(g, 56, 124, 254);
	add_edge(g, 30, 10, 150);
	add_edge(g, 2, 119, 73);
	add_edge(g, 54, 81, 213);
	add_edge(g, 25, 16, 88);
	add_edge(g, 45, 24, 139);
	add_edge(g, 105, 89, 117);
	add_edge(g, 54, 43, 202);
	add_edge(g, 10, 78, 22);
	add_edge(g, 84, 9, 195);
	add_edge(g, 121, 54, 54);
	add_edge(g, 66, 76, 169);
	add_edge(g, 116, 72, 1);
	add_edge(g, 18, 63, 180);
	add_edge(g, 26, 106, 98);
	add_edge(g, 33, 93, 163);
	add_edge(g, 123, 4, 155);
	add_edge(g, 119, 101, 60);
	add_edge(g, 41, 64, 102);
	add_edge(g, 38, 49, 57);
	add_edge(g, 101, 42, 66);
	add_edge(g, 87, 96, 207);
	add_edge(g, 43, 50, 38);
	add_edge(g, 58, 43, 27);
	add_edge(g, 85, 73, 96);
	add_edge(g, 127, 24, 179);
	add_edge(g, 124, 74, 241);
	add_edge(g, 23, 64, 118);
	add_edge(g, 7, 65, 93);
	add_edge(g, 94, 69, 169);
	add_edge(g, 44, 36, 99);
	add_edge(g, 48, 11, 52);
	add_edge(g, 44, 23, 228);
	add_edge(g, 93, 18, 33);
	add_edge(g, 28, 105, 165);
	add_edge(g, 100, 124, 37);
	add_edge(g, 118, 28, 242);
	add_edge(g, 0, 28, 224);
	add_edge(g, 4, 101, 13);
	add_edge(g, 65, 67, 12);
	add_edge(g, 79, 82, 68);
	add_edge(g, 66, 79, 29);
	add_edge(g, 85, 72, 241);
	add_edge(g, 119, 60, 195);
	add_edge(g, 127, 74, 56);
	add_edge(g, 4, 21, 151);
	add_edge(g, 45, 81, 32);
	add_edge(g, 56, 37, 64);
	add_edge(g, 72, 59, 34);
	add_edge(g, 84, 35, 34);
	add_edge(g, 5, 76, 217);
	add_edge(g, 124, 72, 20);
	add_edge(g, 65, 62, 44);
	add_edge(g, 123, 84, 174);
	add_edge(g, 83, 32, 27);
	add_edge(g, 58, 69, 34);
	add_edge(g, 47, 26, 21);
	add_edge(g, 30, 104, 223);
	add_edge(g, 105, 6, 254);
	add_edge(g, 67, 25, 81);
	add_edge(g, 121, 36, 161);
	add_edge(g, 7, 40, 85);
	add_edge(g, 17, 75, 244);
	add_edge(g, 124, 117, 175);
	add_edge(g, 34, 115, 101);
	add_edge(g, 120, 30, 187);
	add_edge(g, 46, 93, 131);
	add_edge(g, 105, 112, 190);
	add_edge(g, 41, 99, 165);
	add_edge(g, 115, 22, 118);
	add_edge(g, 111, 27, 79);
	add_edge(g, 104, 33, 79);
	add_edge(g, 60, 53, 31);
	add_edge(g, 101, 35, 89);
	add_edge(g, 112, 10, 148);
	add_edge(g, 67, 68, 74);
	add_edge(g, 42, 88, 236);
	add_edge(g, 111, 88, 116);
	add_edge(g, 84, 108, 232);
	add_edge(g, 85, 12, 61);
	add_edge(g, 99, 77, 178);
	add_edge(g, 14, 103, 40);
	add_edge(g, 75, 49, 102);
	add_edge(g, 9, 40, 62);
	add_edge(g, 79, 53, 37);
	add_edge(g, 72, 102, 161);
	add_edge(g, 67, 19, 215);
	add_edge(g, 125, 3, 143);
	add_edge(g, 127, 57, 97);
	add_edge(g, 111, 8, 18);
	add_edge(g, 66, 81, 75);
	add_edge(g, 77, 73, 248);
	add_edge(g, 4, 40, 4);
	add_edge(g, 76, 78, 129);
	add_edge(g, 60, 72, 41);
	add_edge(g, 58, 54, 150);
	add_edge(g, 23, 88, 33);
	add_edge(g, 125, 63, 44);
	add_edge(g, 33, 106, 232);
	add_edge(g, 83, 67, 50);
	add_edge(g, 40, 1, 137);
	add_edge(g, 126, 91, 232);
	add_edge(g, 75, 22, 37);
	add_edge(g, 39, 45, 255);
	add_edge(g, 31, 66, 74);
	add_edge(g, 107, 106, 82);
	add_edge(g, 114, 20, 56);
	add_edge(g, 94, 68, 247);
	add_edge(g, 81, 120, 96);
	add_edge(g, 94, 9, 118);
	add_edge(g, 19, 99, 127);
	add_edge(g, 58, 9, 176);
	add_edge(g, 91, 78, 100);
	add_edge(g, 42, 62, 181);
	add_edge(g, 21, 66, 226);
	add_edge(g, 5, 30, 120);
	add_edge(g, 92, 112, 226);
	add_edge(g, 88, 105, 254);
	add_edge(g, 43, 64, 138);
	add_edge(g, 86, 53, 124);
	add_edge(g, 38, 78, 117);
	add_edge(g, 2, 41, 192);
	add_edge(g, 60, 49, 137);
	add_edge(g, 60, 23, 149);
	add_edge(g, 19, 76, 92);
	add_edge(g, 107, 76, 160);
	add_edge(g, 122, 119, 62);
	add_edge(g, 71, 33, 201);
	add_edge(g, 17, 58, 247);
	add_edge(g, 15, 78, 86);
	add_edge(g, 2, 55, 136);
	add_edge(g, 127, 121, 15);
	add_edge(g, 127, 10, 47);
	add_edge(g, 73, 121, 192);
	add_edge(g, 74, 18, 217);
	add_edge(g, 38, 26, 83);
	add_edge(g, 85, 1, 222);
	add_edge(g, 78, 35, 29);
	add_edge(g, 41, 10, 192);
	add_edge(g, 32, 86, 165);
	add_edge(g, 92, 26, 79);
	add_edge(g, 22, 59, 245);
	add_edge(g, 103, 71, 244);
	add_edge(g, 16, 121, 109);
	add_edge(g, 9, 72, 121);
	add_edge(g, 122, 15, 165);
	add_edge(g, 31, 37, 0);
	add_edge(g, 28, 1, 133);
	add_edge(g, 73, 44, 7);
	add_edge(g, 92, 87, 248);
	add_edge(g, 109, 54, 151);
	add_edge(g, 51, 75, 188);
	add_edge(g, 39, 65, 118);
	add_edge(g, 61, 94, 135);
	add_edge(g, 33, 98, 35);
	add_edge(g, 7, 39, 153);
	add_edge(g, 90, 66, 58);
	add_edge(g, 76, 43, 229);
	add_edge(g, 14, 59, 140);
	add_edge(g, 97, 12, 108);
	add_edge(g, 75, 101, 239);
	add_edge(g, 5, 15, 173);
	add_edge(g, 49, 65, 225);
	add_edge(g, 127, 1, 117);
	add_edge(g, 104, 69, 55);
	add_edge(g, 73, 94, 212);
	add_edge(g, 10, 14, 243);
	add_edge(g, 108, 114, 79);
	add_edge(g, 76, 4, 20);
	add_edge(g, 120, 49, 177);
	add_edge(g, 123, 21, 122);
	add_edge(g, 41, 96, 217);
	add_edge(g, 3, 19, 3);
	add_edge(g, 70, 56, 236);
	add_edge(g, 104, 82, 243);
	add_edge(g, 26, 33, 228);
	add_edge(g, 110, 114, 144);
	add_edge(g, 37, 44, 115);
	add_edge(g, 16, 55, 66);
	add_edge(g, 107, 44, 195);
	add_edge(g, 54, 94, 202);
	add_edge(g, 100, 49, 94);
	add_edge(g, 41, 83, 247);
	add_edge(g, 111, 24, 76);
	add_edge(g, 107, 104, 148);
	add_edge(g, 52, 63, 90);
	add_edge(g, 124, 22, 209);
	add_edge(g, 111, 31, 68);
	add_edge(g, 1, 42, 159);
	add_edge(g, 54, 118, 64);
	add_edge(g, 88, 65, 206);
	add_edge(g, 26, 5, 235);
	add_edge(g, 120, 48, 105);
	add_edge(g, 120, 72, 165);
	add_edge(g, 71, 13, 70);
	add_edge(g, 19, 103, 126);
	add_edge(g, 127, 71, 176);
	add_edge(g, 58, 66, 50);
	add_edge(g, 78, 39, 138);
	add_edge(g, 125, 75, 226);
	add_edge(g, 34, 30, 98);
	add_edge(g, 43, 25, 143);
	add_edge(g, 88, 54, 106);
	add_edge(g, 109, 58, 130);
	add_edge(g, 119, 28, 231);
	add_edge(g, 69, 17, 206);
	add_edge(g, 110, 90, 170);
	add_edge(g, 117, 13, 24);
	add_edge(g, 10, 87, 33);
	add_edge(g, 62, 119, 33);
	add_edge(g, 1, 100, 251);
	add_edge(g, 20, 97, 232);
	add_edge(g, 15, 66, 42);
	add_edge(g, 21, 54, 249);
	add_edge(g, 36, 80, 225);
	add_edge(g, 114, 41, 185);
	add_edge(g, 117, 86, 197);
	add_edge(g, 123, 60, 135);
	add_edge(g, 107, 28, 133);
	add_edge(g, 67, 47, 178);
	add_edge(g, 66, 86, 26);
	add_edge(g, 58, 97, 111);
	add_edge(g, 47, 25, 171);
	add_edge(g, 98, 66, 47);
	add_edge(g, 19, 29, 208);
	add_edge(g, 40, 83, 23);
	add_edge(g, 26, 44, 241);
	add_edge(g, 70, 76, 8);
	add_edge(g, 67, 21, 118);
	add_edge(g, 19, 52, 0);
	add_edge(g, 42, 10, 62);
	add_edge(g, 48, 37, 55);
	add_edge(g, 127, 101, 205);
	add_edge(g, 27, 32, 156);
	add_edge(g, 94, 108, 14);
	add_edge(g, 121, 116, 114);
	add_edge(g, 87, 20, 11);
	add_edge(g, 11, 16, 176);
	add_edge(g, 102, 56, 103);
	add_edge(g, 63, 9, 86);
	add_edge(g, 80, 37, 166);
	add_edge(g, 20, 10, 71);
	add_edge(g, 82, 101, 80);
	add_edge(g, 104, 16, 167);
	add_edge(g, 79, 104, 4);
	STOP_MEASURE(DGI_GPIO3);
	
	START_MEASURE(DGI_GPIO2);
	dijkstra(g, 0, 127);
	STOP_MEASURE(DGI_GPIO2);
	
	#ifdef DEBUG_PRINT
	print_path(g, 127);
	#endif // DEBUG_PRINT
	
	END_MEASUREMENT;
	
	return 0;
}
