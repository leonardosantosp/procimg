#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../utils/imagelib.h"

image negative(image In) {
    image Out = img_clone(In);
    for (int i = 0; i < In->nr * In->nc; i++)
        Out->px[i] = In->ml - In->px[i];
    return Out;
}

image linearization_binary(image In) {
    float T[In->ml + 1];
    image Out = img_clone(In);
    for (int i = 0; i < In->ml + 1; i++) {
        T[i] = i < 160 ? 0 : 1;
    }
    for (int i = 0; i < In->nr * In->nc; i++)
        Out->px[i] = T[In->px[i]];
    return Out;
}

image linearization_binary2(image In) {
    float T[In->ml + 1];
    image Out = img_clone(In);
    for (int i = 0; i < In->ml + 1; i++) {
        T[i] = i < 6 ? 0 : 1;
    }
    for (int i = 0; i < In->nr * In->nc; i++)
        Out->px[i] = T[In->px[i]];
    return Out;
}

int find(int parent[], int i) {
    while (parent[i] != i)
        i = parent[i];
    return i;
}

void Union(int parent[], int i, int j) {
    int x = find(parent, i);
    int y = find(parent, j);
    parent[y] = x;
}

int countLabels(image img, int labels[]) {
    int uniqueCount = 0;
    int labelUsed[img->nc * img->nr];
    for (int i = 0; i < img->nc * img->nr; i++)
        labelUsed[i] = 0;
    for (int i = 0; i < img->nc * img->nr; i++) {
        int rootLabel = find(labels, img->px[i]);
        if (labelUsed[rootLabel] == 0 && rootLabel != 0) {
            labelUsed[rootLabel] = 1;
            uniqueCount++;
        }
    }
    return uniqueCount;
}

void label(image In) {
    int nr = In->nr;
    int nc = In->nc;
    int *px = In->px;
    int numLabel = 0;
    int parent[1000];

    for (int i = 0; i < 1000; i++)
        parent[i] = i;

    for (int i = 1; i < nr; i++) {
        for (int j = 1; j < nc; j++) {
            int p = px[i * nc + j];
            int r = px[(i - 1) * nc + j];
            int t = px[i * nc + j - 1];
            if (p != 0) {
                if (r == 0 && t == 0)
                    px[i * nc + j] = ++numLabel;
                if (r != 0 && t == 0)
                    px[i * nc + j] = r;
                if (r == 0 && t != 0)
                    px[i * nc + j] = t;
                if (r != 0 && t != 0 && t == r)
                    px[i * nc + j] = r;
                if (r != 0 && t != 0 && t != r) {
                    px[i * nc + j] = t;
                    Union(parent, r, t);
                }
            }
        }
    }

    for (int i = 0; i < nr * nc; i++)
        In->px[i] = find(parent, In->px[i]);

    In->ml = numLabel;
    printf("#componentes: %d\n", countLabels(In, parent));
}

int min3(int a, int b, int c) {
    if (a < b && a < c) return a;
    if (b < c) return b;
    return c;
}

int distance(image In) {
    int max_distance = -1;
    int nr = In->nr;
    int nc = In->nc;
    int *px = In->px;

    for (int i = 1; i < nr - 1; i++) {
        for (int j = 1; j < nc - 1; j++) {
            int p = px[i * nc + j];
            int a = px[(i - 1) * nc + j];
            int b = px[i * nc + j - 1];
            if (p != 0) {
                px[i * nc + j] = (a + 1) < (b + 1) ? (a + 1) : (b + 1);
            }
        }
    }

    for (int i = nr - 2; i > 0; i--) {
        for (int j = nc - 2; j > 0; j--) {
            int p = px[i * nc + j];
            int a = px[i * nc + j + 1];
            int b = px[(i + 1) * nc + j];
            if (p != 0) {
                px[i * nc + j] = min3(a + 1, b + 1, p);
                if (max_distance < px[i * nc + j]) {
                    max_distance = px[i * nc + j];
                }
            }
        }
    }
    return max_distance;
}

int main(int argc, char *argv[]) {
    char nameIn[100], nameOut[100], cmd[110];
    image In, Out;

    img_name(argv[1], nameIn, nameOut, GRAY, GRAY);

    In = img_get(nameIn, GRAY);
    Out = negative(In);
    Out = linearization_binary(Out);
    Out->ml = distance(Out);
    Out = linearization_binary2(Out);
    label(Out);
    img_put(Out, nameOut, GRAY);

    sprintf(cmd, "%s %s &", VIEW, nameOut);
    system(cmd);

    img_free(In);
    img_free(Out);

    return 0;
}
