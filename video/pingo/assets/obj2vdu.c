// File: obj2vdu.c
// Purpose: Convert OBJ files to Agon VDU commands for Pingo 3D
// By: Curtis Whitley
//
// Apache License
//
// The following OBJ lines are used; all others are ignored.
//
// # <comment ...>
// o <object name>
// g <group name>
// v <x> <y> <z>
// vt <u> <v>
// f <v1/vt1[/vn1]> [<v2/vt2[/vn2]> ...]

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VERTEXES    100000
#define MAX_TEX_COORDS  100000

typedef struct {
    double x;
    double y;
    double z;
} Vertex;

typedef struct {
    double u;
    double v;
} TexCoord;

int num_files;
int num_vertexes;
int num_tex_coords;
Vertex vertexes[MAX_VERTEXES];
TexCoord tex_coords[MAX_TEX_COORDS];

int convert(FILE* fin, FILE* fout) {
    char line[200];
    char *cmd, *tkn;
    double x, y, z, u, v, a, max_coord;
    int iv, ivt, ivn;
    num_vertexes = 0;
    num_tex_coords = 0;
    max_coord = -99999999.0;

    while (fgets(line, sizeof(line)-1, fin)) {
        cmd = strtok(line, " ");
        if (!strcasecmp(cmd, "o")) {
            tkn = strtok(NULL, "\r\n");
            printf("Object: %s\n", tkn);
        } else if (!strcasecmp(cmd, "g")) {
            tkn = strtok(NULL, "\r\n");
            printf("Group: %s\n", tkn);
        } else if (!strcasecmp(cmd, "v")) {
            tkn = strtok(NULL, " ");
            x = atof(tkn);
            tkn = strtok(NULL, " ");
            y = atof(tkn);
            tkn = strtok(NULL, " ");
            z = atof(tkn);
            if (num_vertexes < MAX_VERTEXES) {
                Vertex* pv = &vertexes[num_vertexes++];
                pv->x = x;
                pv->y = y;
                pv->z = z;
                a = fabs(x);
                if (a > max_coord) max_coord = a;
                a = fabs(y);
                if (a > max_coord) max_coord = a;
                a = fabs(z);
                if (a > max_coord) max_coord = a;
            } else {
                printf("Too many vertexes!\n");
                return 3;
            }
        } else if (!strcasecmp(cmd, "vt")) {
            tkn = strtok(NULL, " ");
            u = atof(tkn);
            tkn = strtok(NULL, " ");
            v = atof(tkn);
            if (num_tex_coords < MAX_TEX_COORDS) {
                TexCoord* pt = &tex_coords[num_tex_coords++];
                pt->u = u;
                pt->v = v;
            } else {
                printf("Too many texture coordinates!\n");
                return 4;
            }
        } else if (!strcasecmp(cmd, "f")) {
            
        }
    }
}

int main(int argc, const char* argv[]) {
    printf("OBJ-to-VDU File Convertor V0.1\n");
    if (argc < 2) {
        printf("Usage: obj2vdu file1 [file2, ...]\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        FILE* fin = fopen(argv[i], "r");
        if (fin) {
            char oname[128];
            int len = strlen(argv[i]);
            strcpy(oname, argv[i]);
            if (len >= 4 && !strcasecmp(&oname[len-4], ".obj")) {
                strcpy(&oname[len-4], ".vdu");
            } else {
                strcat(oname, ".vdu");
            }
            FILE* fout = fopen(oname, "w");
            if (fout) {
                printf("Converting '%s' to '%s'\n", argv[i], oname);
                int rc = convert(fin, fout);
                fclose(fout);
                fclose(fin);
                return rc;
            } else {
                printf("Cannot open '%s'\n", oname);
                fclose(fin);
                return 2;
            }
        } else {
            printf("Cannot open '%s'\n", argv[i]);
            return 1;
        }

    }        
}