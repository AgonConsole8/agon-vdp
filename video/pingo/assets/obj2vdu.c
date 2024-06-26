// File: obj2vdu.c
// Purpose: Convert OBJ files to Agon VDU commands for Pingo 3D
// By: Curtis Whitley
//
// Apache License
//
// The following OBJ lines are used; all others are ignored.
//
// o <object name>
// g <group name>
// v <x> <y> <z>
// vt <u> <v>
// f <v1/vt1[/vn1]> [<v2/vt2[/vn2]> ...]
// usemtl <material name>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_VERTEXES    100000
#define MAX_TEX_COORDS  100000
#define MAX_FACES       100000
#define MAX_POLY_PTS    8

typedef struct {
    double x;
    double y;
    double z;
} Vertex;

typedef struct {
    double u;
    double v;
} TexCoord;

typedef struct {
    int ivertex;
    int itexture;
    int inormal;
} PolyPoint;

typedef struct {
    int num_pts;
    PolyPoint points[MAX_POLY_PTS];
} Face;

int num_files;
int num_vertexes;
int num_tex_coords;
int num_faces;
Vertex vertexes[MAX_VERTEXES];
TexCoord tex_coords[MAX_TEX_COORDS];
Face faces[MAX_FACES];
char line[200];
char obj_name[50];
char grp_name[50];
char *cmd, *tkn;
double x, y, z, u, v, a, max_coord;
int iv, ivt, ivn, line_nbr;
bool in_vertexes;
bool in_tex_coords;
bool in_faces;

void extract_point(const char* pt_info, Face* face) {
    PolyPoint *pt = &face->points[face->num_pts++];
    pt->ivertex = 0;
    pt->itexture = 0;
    pt->inormal = 0;
    while (isdigit(*pt_info)) {
        pt->ivertex = (pt->ivertex * 10) + (*pt_info - '0');
        pt_info++;
    }
    pt_info++;
    while (isdigit(*pt_info)) {
        pt->itexture = (pt->itexture * 10) + (*pt_info - '0');
        pt_info++;
    }
    pt_info++;
    while (isdigit(*pt_info)) {
        pt->inormal = (pt->inormal * 10) + (*pt_info - '0');
        pt_info++;
    }
}

void clean_name(char* name) {
    while (*name) {
        if (!(isalnum(*name) ||
            (*name == '-') ||
            (*name == '_') ||
            (*name == '.'))) {
            *name = '_';
        }
        name++;
    }
}

void note_vertexes() {
    if (in_vertexes) {
        printf("[%06i] End of %i vertexes\n", line_nbr-1, num_vertexes);
        in_vertexes = false;
    }
}

void note_tex_coords() {
    if (in_tex_coords) {
        printf("[%06i] End of %i texture coordinates\n", line_nbr-1, num_tex_coords);
        in_tex_coords = false;
    }
}

void note_faces() {
    if (in_faces) {
        printf("[%06i] End of %i faces\n", line_nbr-1, num_faces);
        in_faces = false;
    }
}

int write_object(FILE* fout) {
    note_vertexes();
    note_tex_coords();
    note_faces();

    num_vertexes = 0;
    num_tex_coords = 0;
    num_faces = 0;
    max_coord = -99999999.0;
    *obj_name = 0;
    *grp_name = 0;
}

int convert(FILE* fin, FILE* fout) {
    int rc;
    in_vertexes = false;
    in_tex_coords = false;
    in_faces = false;
    num_vertexes = 0;
    num_tex_coords = 0;
    num_faces = 0;
    max_coord = -99999999.0;
    *obj_name = 0;
    *grp_name = 0;
    line_nbr = 0;

    while (fgets(line, sizeof(line)-1, fin)) {
        line_nbr++;
        int len = strlen(line);
        while (len && isspace(line[len-1])) {
            line[--len] = 0;
        }
        cmd = strtok(line, " ");
        if (!strcasecmp(cmd, "o")) {
            rc = write_object(fout);
            if (rc) return rc;
            tkn = strtok(NULL, "\r\n");
            strcpy(obj_name, tkn);
            clean_name(obj_name);
            printf("[%06i] Object: %s => %s\n", line_nbr, tkn, obj_name);
        } else if (!strcasecmp(cmd, "g")) {
            note_vertexes();
            note_tex_coords();
            note_faces();
            tkn = strtok(NULL, "\r\n");
            strcpy(grp_name, tkn);
            clean_name(grp_name);
            printf("[%06i] Group: %s => %s\n", line_nbr, tkn, grp_name);
        } else if (!strcasecmp(cmd, "v")) {
            if (!num_vertexes) {
                printf("[%06i] Start of vertexes\n", line_nbr);
            }
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
                in_vertexes = true;
            } else {
                printf("Too many vertexes!\n");
                return 3;
            }
        } else if (!strcasecmp(cmd, "vt")) {
            note_vertexes();
            if (!num_tex_coords) {
                printf("[%06i] Start of texture coordinates\n", line_nbr);
            }
            tkn = strtok(NULL, " ");
            u = atof(tkn);
            tkn = strtok(NULL, " ");
            v = atof(tkn);
            if (num_tex_coords < MAX_TEX_COORDS) {
                TexCoord* pt = &tex_coords[num_tex_coords++];
                pt->u = u;
                pt->v = v;
                in_tex_coords = true;
            } else {
                printf("Too many texture coordinates!\n");
                return 4;
            }
        } else if (!strcasecmp(cmd, "f")) {
            note_vertexes();
            note_tex_coords();
            if (!num_faces) {
                printf("[%06i] Start of faces\n", line_nbr);
            }
            Face* face = &faces[num_faces++];
            face->num_pts = 0;
            while (tkn = strtok(NULL, " ")) {
                char pt_info[50];
                strcpy(pt_info, tkn);
                extract_point(tkn, face);
            }
            in_faces = true;
        } else if (!strcasecmp(cmd, "usemtl")) {
        } else {
            note_vertexes();
            note_tex_coords();
            note_faces();
        }
    }
    write_object(fout);
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