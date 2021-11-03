#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * md2.c -- md2 model loader
 * last modification: aug. 14, 2007
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * gcc -Wall -ansi -lGL -lGLU -lglut md2.c -o md2
*
* md2modeldef made by CBM from ZDoom and Doomworld forums in 2019
 */

 int noskin=0;
 int skincount=0;
char mdlskinsname[100] = "";

char framenametmp[100] = "";
char framenamesets[10000] = "";


/* Vector */
typedef float vec3_t[3];

char skinname[100] = "";
char mdfilename[100] = "";
char modelname[100] = "";
FILE *modeldef;
int tris;


/* MD2 header */
struct md2_header_t
{
  int ident;
  int version;

  int skinwidth;
  int skinheight;

  int framesize;

  int num_skins;
  int num_vertices;
  int num_st;
  int num_tris;
  int num_glcmds;
  int num_frames;

  int offset_skins;
  int offset_st;
  int offset_tris;
  int offset_frames;
  int offset_glcmds;
  int offset_end;
};

/* Texture name */
struct md2_skin_t
{
  char name[64];
};

/* Texture coords */
struct md2_texCoord_t
{
  short s;
  short t;
};

/* Triangle info */
struct md2_triangle_t
{
  unsigned short vertex[3];
  unsigned short st[3];
};

/* Compressed vertex */
struct md2_vertex_t
{
  unsigned char v[3];
  unsigned char normalIndex;
};

/* Model frame */
struct md2_frame_t
{
  vec3_t scale;
  vec3_t translate;
  char name[16];
  struct md2_vertex_t *verts;
};

/* GL command packet */
struct md2_glcmd_t
{
  float s;
  float t;
  int index;
};

/* MD2 model structure */
struct md2_model_t
{
  struct md2_header_t header;

  struct md2_skin_t *skins;
  struct md2_texCoord_t *texcoords;
  struct md2_triangle_t *triangles;
  struct md2_frame_t *frames;
  int *glcmds;

  //GLuint tex_id;
};

// FILE *decorate;  // if the tool becomes expanded with ability to write decorate files

/* Table of precalculated normals */
vec3_t anorms_table[162] = {
#include "anorms.h"
};

/*** An MD2 model ***/
struct md2_model_t md2file;

/**
 * sub string
 */

void subString(char * string, char * sub, int position, int length)
{
   int c = 0;
   while (c < length)
   {
      sub[c] = string[position+c-1];
      c++;
   }
   sub[c] = '\0';

}

/*** get size ***/
int size(char *ptr)
{
    //variable used to access the subsequent array elements.
    int offset = 0;
    //variable that counts the number of elements in your array
    int count = 0;

    //While loop that tests whether the end of the array has been reached
    while (*(ptr + offset) != '\0')
    {
        //increment the count variable
        ++count;
        //advance to the next element of the array
        ++offset;
    }
    //return the size of the array
    return count;
}

/*** get pic ***/
int picindex(char *ptr)
{
    //variable used to access the subsequent array elements.
    int offset = 0;
    //variable that counts the number of elements in your array
    int count = 0;

    int done = 0;
    //While loop that tests whether the end of the array has been reached
    //while (*(ptr + offset) != '\0')
    while (done<2)
    {
        if (*(ptr + offset) == '/') {done++;}
        if (*(ptr + offset) == '\\') {done++;}
        //increment the count variable
        ++count;
        //advance to the next element of the array
        ++offset;
    }
    //return the size of the array
    return count;
}

/*** get dot ***/
int dotindex(char *ptr)
{
    //variable used to access the subsequent array elements.
    int offset = 0;
    //variable that counts the number of elements in your array
    int count = 0;

    int done = 0;
    //While loop that tests whether the end of the array has been reached
    while (*(ptr + offset) != '.')
    {
        //increment the count variable
        ++count;
        //advance to the next element of the array
        ++offset;
    }
    //return the size of the array
    return count;
}

/*** get last slash ***/
int slashindex(char *ptr)
{
    //variable used to access the subsequent array elements.
    int offset = 0;
    //variable that counts the number of elements in your array
    int count = 0;

    int done = 0;
    //While loop that tests whether the end of the array has been reached

    while (*(ptr + offset) != '\0')
    {
        //increment the count variable
        ++count;
        //advance to the next element of the array
        ++offset;

        if (*(ptr + offset) == '/') {done=count;}
        if (*(ptr + offset) == '\\') {done=count;}

    }
    //return the size of the array
    return done;
}


/*** strip path ***/
int strip(char * source, char * destination)
{
    int end = 0;
    int begin = slashindex(source);
    end = dotindex(source);
    subString(source,destination,begin+2,end-begin);
    destination[size(destination)-1] = '\0';
    return end;
}

/*** get file tris ***/
int makefile(char * string)
{
    int tmp, picture,finalloc, namebegin, slashcount, result, endIndex, lastIndex, i;
    char temp[100] = "";
    char temp2[100] = "";
    char temp3[100] = "";

      printf("standard md2 open... \n");

    /* Finds the last non white space character */
    i = 0;
    endIndex = 0;
    result = 0;
    slashcount = 0;

    picture = picindex(string);

    subString(string,temp3,picture+1,size(string));

    while(slashcount<2)
    {

        if(string[i] == '.')
        {
            slashcount=2;
        }
        else if (string[i] == '\0')
        {
            slashcount=2;
        }
        else if (string[i] == '/')
        {
           if (slashcount==0){namebegin=i;}
           slashcount++;
        }
        else if (string[i] == '\\')
        {
           if (slashcount==0){namebegin=i;}
           slashcount++;
        }

        if (slashcount<2)
        {
            //printf("char:  %c  \n", string[i]);
            temp[i]=string[i];
        }
        else
        {
            endIndex = i;
            temp[i]='\0';
        }

        i++;
    }

    printf("getting size... \n");

    result = endIndex;
    subString(temp,temp2,namebegin+2,size(temp));

    /* Mark the next character to last non white space character as NULL */

    printf("temp : %s  \n", temp);
    printf("temp2: %s  \n", temp2);
    printf("temp3: %s  \n", temp3);

    strip(string,temp2);

    printf("string: %s  \n", string);
    printf("temp2: %s  \n", temp2);

    if (size(temp2)<1)
    {

      if (size(mdfilename)<1) { strcat(temp2,"tris"); }
      else
      {
        tmp = slashindex(mdfilename);
        subString(mdfilename,temp2,tmp,size(mdfilename)-tmp);
        printf("newname: %s  \n", temp2);
      }

    }

    //detect duplicate sprite names

    strcat(modelname,"tris.md2");

    strcat(mdfilename,"modeldef.");
    strcat(mdfilename,temp2);

    printf("creating filename... %s \n",mdfilename);

    modeldef = fopen(mdfilename, "w");

    fprintf(modeldef, "MODEL %s \n", temp2);
    fprintf(modeldef, "{ \n");
    fprintf(modeldef, "Path \"Models/%s\" \n",temp2);

    if (noskin==1)
    {
    fprintf(modeldef, "SKIN 0 \"skin.pcx\" \n");
    }
    else
    {
    fprintf(modeldef, "SKIN 0 \"%s\" \n",temp3);
    }

    printf("continue... \n");

    return result;
}

/*** get file ***/
int makefile2(char * string)
{
    int picture,finalloc, namebegin, slashcount, result, endIndex, lastIndex, i;
    char temp[100] = "";
    char temp2[100] = "";
    char temp3[100] = "";
    char temp4[100] = "";
    char temp5[100] = "";

    strcat(temp5,"_");
    strcat(temp5,string);

   printf("custom md2 open... %s \n",string);

    /* Finds the last non white space character */
    i = 0;
    endIndex = 0;
    result = 0;
    slashcount = 0;

    picture = picindex(string);

    subString(string,temp3,picture+1,size(string));
    subString(string,temp4,1,size(string)-4);

    while(slashcount<2)
    {

        if(string[i] == '.')
        {
            slashcount=2;
        }
        i++;
    }

    printf("getting size... \n");
    result = endIndex;

    /* Mark the next character to last non white space character as NULL */

    printf("temp : %s  \n", temp);
    printf("temp2: %s  \n", temp2);
    printf("temp3: %s  \n", temp3);
    printf("temp4: %s  \n", temp4);

    strip(temp5, temp4);

    strcat(mdfilename,"modeldef.");
    strcat(mdfilename,temp4);
    strcat(mdfilename,"\0");

    printf("string: %s  \n", string);
    printf("temp4: %s  \n", temp4);


    printf("creating filename... %s \n",mdfilename);

    modeldef = fopen(mdfilename, "w");

    fprintf(modeldef, "// custom md2 file...  \n");
    fprintf(modeldef, "MODEL %s \n", temp4);
    fprintf(modeldef, "{ \n");
    fprintf(modeldef, "Path \"Models/%s\" \n",temp4);

    if (skincount>0)
    {
        picture = picindex(mdlskinsname);

    subString(mdlskinsname,temp3,picture+1,size(mdlskinsname));

       fprintf(modeldef, "SKIN 0 \"%s\"  \n", temp3);
    }
    else
    {
       fprintf(modeldef, "SKIN 0 \"%s.pcx\" \n",temp4);
    }

    printf("continue... \n");

    strcat(modelname,temp4);
    strcat(modelname,".md2");

    return result;
}


/*** get path ***/
int getindex(char * string)
{
    int picture,finalloc, namebegin, slashcount, result, endIndex, lastIndex, i;
    char temp[100] = "";
    char temp2[100] = "";
    char temp3[100] = "";

    /* Finds the last non white space character */
    i = 0;
    endIndex = 0;
    result = 0;
    slashcount = 0;

    picture = picindex(string);

    subString(string,temp3,picture+1,size(string));

    while(slashcount<2)
    {

        if(string[i] == '.')
        {
            slashcount=2;
        }
        else if (string[i] == '\0')
        {
            slashcount=2;
        }
        else if (string[i] == '/')
        {
           if (slashcount==0){namebegin=i;}
           slashcount++;
        }
        else if (string[i] == '\\')
        {
           if (slashcount==0){namebegin=i;}
           slashcount++;
        }

        if (slashcount<2)
        {
            temp[i]=string[i];
        }
        else
        {
            endIndex = i;
            temp[i]='\0';
        }

        i++;
    }

    printf("getting size... \n");

    result = endIndex;
    subString(temp,temp2,namebegin+2,size(temp));

    /* Mark the next character to last non white space character as NULL */
    printf("temp : %s  \n", temp);
    printf("temp2: %s  \n", temp2);
    printf("temp3: %s  \n", temp3);

    printf("continue... \n");

    return result;
}


/**
 * Remove trailing whitespace characters from a string
 */
int trimTrailing(char * string)
{
    int result, endIndex, lastIndex, i;
    char temp[100] = "";

    /* Finds the last non white space character */
    i = 0;
    while(string[i] != '\0')
    {
        if(string[i] != '0' && string[i] != '1' && string[i] != '2' && string[i] != '3' && string[i] != '4' && string[i] != '5' &&
           string[i] != '6' && string[i] != '7' && string[i] != '8' && string[i] != '9' && string[i] != '\t' && string[i] != '\n')
        {
            lastIndex = i;
        }

        i++;
    }
    endIndex  = i;

    result = 0;
    if (lastIndex+2<endIndex)
    {
      subString(string,temp,lastIndex+2,endIndex);
      result = atoi(temp);
    }

    /* Mark the next character to last non white space character as NULL */
    string[lastIndex + 1] = '\0';

    printf("temp:  %s  \n", temp);

    return result;
}

/*** getsub ***/

int getSub(const char *str, const char *sub)
{
  char *p1, *p2, *p3;
  int i=0,j=0,flag=0;

  p1 = str;
  p2 = sub;

  for(i = 0; i<strlen(str); i++)
  {
    if(*p1 == *p2)
      {
          p3 = p1;
          for(j = 0;j<strlen(sub);j++)
          {
            if(*p3 == *p2)
            {
              p3++;p2++;
            }
            else
              break;
          }
          p2 = sub;
          if(j == strlen(sub))
          {
             flag = 1;
            printf("\nSubstring found at index : %d\n",i);
          }
      }
    p1++;
  }
  return flag;
}

/*** compare **/
int StringCompare(const char *s1, const char *s2)
{ // returns 0 if the strings are equivalent, 1 if they're not
  while( (*s1!=0) && (*s2!=0) )
  { // loop until either string runs out
     if(*s1!=*s2) return 1; // check if they match
     s1++; // skip to next character
     s2++;
  }
  if( (*s1==0) && (*s2==0) ) // did both strings run out at the same length?
      return 0;
  return 1; // one is longer than the other
}


/**
 * Load an MD2 model from file.
 *
 * Note: MD2 format stores model's data in little-endian ordering.  On
 * big-endian machines, you'll have to perform proper conversions.
 */
int
ReadMD2Model (const char *filename, struct md2_model_t *mdl)
{
  FILE *fp;

  char oldset[100] = "";
  char setname[100] = "";
  char setdisp[100] = "";
  char olddisp[100] = "";
  char framename[100] = "";
  char letter;
  char compname[100] = "";
  char tempname[100] = "";

  int write=1;
  int i;
  int oldval;
  int newval;
  int setnum;
  int localletter;

  int globalframe=0;
  int localframe=0;
  int nrframe=0;
  int oldnrf=0;
  int oldframe=0;
  int localoffset=0;
  int written=0;

  fp = fopen (filename, "rb");
  if (!fp)
    {
      fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
      return 0;
    }

    i = slashindex(filename);
    if (i>0)
    {
      printf("slashindex : %d \n",i);
      subString(filename,mdfilename,1,i+1);
      i = 0;
      printf("file : %s \n",filename);
      printf("found path : %s \n",mdfilename);
      strcat(compname, mdfilename);

      subString(mdfilename,tempname,1,size(mdfilename)-2);
      i = slashindex(tempname);
      subString(tempname,skinname,i+1,(size(tempname)-i));

    }
    else
    {
      strcat(skinname, "\tris");
    }

    strcat(compname, "tris.md2");

tris=StringCompare(filename,compname);

if (tris==0)
{
      printf("standard md2... \n");

}
else
{
  printf("custom md2... \n");

}


  /* Read header */
  fread (&mdl->header, 1, sizeof (struct md2_header_t), fp);

  if ((mdl->header.ident != 844121161) ||
      (mdl->header.version != 8))
    {
      fprintf (stderr, "Error: bad version or identifier\n");

      if (mdl->header.ident = 1367369843)
        {

      fprintf (stderr, "Quake 3 model identifier\n");

        }

      fclose (fp);
      return 0;

    }

  /* Memory allocations */
  mdl->skins = (struct md2_skin_t *)
    malloc (sizeof (struct md2_skin_t) * mdl->header.num_skins);
  mdl->texcoords = (struct md2_texCoord_t *)
    malloc (sizeof (struct md2_texCoord_t) * mdl->header.num_st);
  mdl->triangles = (struct md2_triangle_t *)
    malloc (sizeof (struct md2_triangle_t) * mdl->header.num_tris);
  mdl->frames = (struct md2_frame_t *)
    malloc (sizeof (struct md2_frame_t) * mdl->header.num_frames);
  mdl->glcmds = (int *)malloc (sizeof (int) * mdl->header.num_glcmds);

  /* Read model data */
  fseek (fp, mdl->header.offset_skins, SEEK_SET);
  fread (mdl->skins, sizeof (struct md2_skin_t),
	 mdl->header.num_skins, fp);

  char dirname[100];

  skincount=mdl->header.num_skins;

  if (skincount>0)
  {
       strncpy(mdlskinsname, (mdl->skins[0].name), 100);
  }

  if (tris==0)
  {
if (mdl->header.num_skins==1)
        {
       printf("skin name: %s  \n", mdl->skins[0].name);
       strncpy(dirname, (mdl->skins[0].name), 100);
       makefile(dirname);
	 }
else
{
    printf("required number of skins is 1 for Quake 2 player models... this model has %d skins \n", skincount);

      noskin=1;

       printf("skin name suggestion: skin.pcx  \n");
       strncpy(dirname, skinname, 100);
       makefile(dirname);
}

  }
else
{
    makefile2(filename);
}
    printf("completing header... \n");

	   if (tris==0)
       {
	     fprintf(modeldef, "MODEL 0 \"tris.md2\" \n \n");

	     fprintf(modeldef, "SKIN 1 \"weapon.pcx\" \n");
	     fprintf(modeldef, "MODEL 1 \"weapon.md2\" \n \n");

	     fprintf(modeldef, "Scale 1.2 1.2 1.2 \n");
	     fprintf(modeldef, "zoffset 20 \n \n");
       }
       else
       {
         fprintf(modeldef, "MODEL 0 \"%s\" \n",modelname);
         fprintf(modeldef, "Scale 1.0 1.0 1.0 \n");
       }

  fseek (fp, mdl->header.offset_st, SEEK_SET);
  fread (mdl->texcoords, sizeof (struct md2_texCoord_t),
	 mdl->header.num_st, fp);

  fseek (fp, mdl->header.offset_tris, SEEK_SET);
  fread (mdl->triangles, sizeof (struct md2_triangle_t),
	 mdl->header.num_tris, fp);

  fseek (fp, mdl->header.offset_glcmds, SEEK_SET);
  fread (mdl->glcmds, sizeof (int), mdl->header.num_glcmds, fp);

  setnum=0;

      printf("writing frames... \n");

     i = 0;

  /* Read frames */
  fseek (fp, mdl->header.offset_frames, SEEK_SET);
  for (i = 0; i < mdl->header.num_frames; ++i)
    {
        globalframe++;
        localframe++;

      /* Memory allocation for vertices of this frame */
      mdl->frames[i].verts = (struct md2_vertex_t *)
	malloc (sizeof (struct md2_vertex_t) * mdl->header.num_vertices);

      /* Read frame data */

      fread (mdl->frames[i].scale, sizeof (vec3_t), 1, fp);
      fread (mdl->frames[i].translate, sizeof (vec3_t), 1, fp);
      fread (mdl->frames[i].name, sizeof (char), 16, fp);

     strncpy(oldset, setname, 100);
     strncpy(olddisp, oldset, 100);
     strncpy(setdisp, (mdl->frames[i].name), 100);
     strncpy(setname, (mdl->frames[i].name), 100);

     oldval = 0;  // how to get vals to detect jumps???
     newval = 0;

     newval = trimTrailing(setdisp);
     oldval = trimTrailing(olddisp);

           printf("writing frame... \n");

                   subString(setname,framename,1,3);


        if (strcmp(olddisp, setdisp) == 0)
        {
            printf("same set \n");
        }
        else
        {

            write=1;

  if (StringCompare(framename,"poi")==0) {write=0;}
  if (StringCompare(framename,"wav")==0) {write=0;}
  if (StringCompare(framename,"tau")==0) {write=0;}
  if (StringCompare(framename,"sal")==0) {write=0;}
  if (StringCompare(framename,"fli")==0) {write=0;}
  if (StringCompare(framename,"jum")==0) {write=0;}
  if (StringCompare(framename,"dea")==0) {write=1;}
  if (StringCompare(framename,"crd")==0) {write=1;}
  if (localframe>9) {write=0;}

  if (write==1)
           {
               written++;
               printf("write: %s  \n", framename);
           }
else
{
               printf("skip: %s  \n", framename);

}
            localoffset=0;
            localframe=0;
            localletter=0;
            setnum++;
            printf("new set: %d  \n", setnum);

            subString(mdl->frames[i].name,framenametmp,1,3);

            if (getSub(framenamesets,framenametmp)==1)
            {
              subString(mdl->frames[i].name,framenametmp,2,3);
              if (getSub(framenamesets,framenametmp)==1)
              {
              subString(mdl->frames[i].name,framenametmp,3,3);
              }
              write=2;
              strcat(framenamesets, framenametmp);
            }
            else
            {
            strcat(framenamesets, framename);
            }

            fprintf(modeldef, "// %s set %d \n",mdl->frames[i].name,setnum);

        }

        // modelnumber and A,B,C,D,E,F,G,H,I,J
        // 0 is tris and 1 is weapon

        nrframe=0;
        if (localframe>89) {nrframe=9;}//localletter=0;}
        if (localframe<90) {nrframe=8;}//localletter=0;}
        if (localframe<80) {nrframe=7;}//localletter=0;}
        if (localframe<70) {nrframe=6;}//localletter=0;}
        if (localframe<60) {nrframe=5;}//localletter=0;}
        if (localframe<50) {nrframe=4;}//localletter=0;}
        if (localframe<40) {nrframe=3;}//localletter=0;}
        if (localframe<30) {nrframe=2;}//localletter=0;}
        if (localframe<20) {nrframe=1;}//localletter=0;}
        if (localframe<10) {nrframe=0;}//localletter=0;}

        if (newval > oldval+2)
            {
                printf("val increase above 1 \n"); localoffset++;
            }

        nrframe+=localoffset;

        if (nrframe != oldnrf) { localletter=0; }
        oldnrf = nrframe;

        localletter++;

//  localframe

                if (localletter==1) {letter='A';}
                if (localletter==2) {letter='B';}
                if (localletter==3) {letter='C';}
                if (localletter==4) {letter='D';}
                if (localletter==5) {letter='E';}
                if (localletter==6) {letter='F';}
                if (localletter==7) {letter='G';}
                if (localletter==8) {letter='H';}
                if (localletter==9) {letter='I';}
                if (localletter==10) {letter='J';}
                if (localletter==11) {letter='K';}
                if (localletter==12) {letter='L';}
                if (localletter==13) {letter='M';}
                if (localletter==14) {letter='N';}
                if (localletter==15) {letter='O';}
                if (localletter==16) {letter='P';}
                if (localletter==17) {letter='Q';}
                if (localletter==18) {letter='R';}
                if (localletter==19) {letter='S';}
                if (localletter==20) {letter='T';}
                if (localletter==21) {letter='U';}
                if (localletter==22) {letter='V';}
                if (localletter==23) {letter='W';}
                if (localletter==24) {letter='X';}
                if (localletter==25) {letter='Y';}
                if (localletter==26) {letter='Z';}

  printf("frame letter:  %c  \n", letter);

  printf("--------------------------------------- \n");

  printf("local:  %d  \n", localframe);
  printf("global:  %d  \n", globalframe);

  printf("sprite val new:  %d  \n", newval);
  printf("sprite val old:  %d  \n", oldval);

  printf("sprite name:  %s%d \n", framename,nrframe);

  printf("frame old:  %s  \n", olddisp);
  printf("frame set:  %s  \n", setdisp);
  printf("frame nr:   %d  \n", i);
  printf("frame name: %s  \n", mdl->frames[i].name);
  printf("--------------------------------------- \n");

  if (nrframe>0) {write=0;}

  //poi,wav,tau,sal,fli,jum

 //   || () || (framename='wav') || (framename='tau') || (framename='sal') || (framename='fli') || (framename='jum'))

 	   if (tris==0)
       {

  if (write==1)
  {
  fprintf(modeldef, "frame %s%d %c 0 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  fprintf(modeldef, "frame %s%d %c 1 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  printf("written (%d of 10) sprite name:  %s%d \n", written,framename,nrframe);
  }
  else if (write==2)
  {
  fprintf(modeldef, "frame %s%d %c 0 \"%s\" \n", framenametmp,nrframe,letter, mdl->frames[i].name);
  fprintf(modeldef, "frame %s%d %c 1 \"%s\" \n", framenametmp,nrframe,letter, mdl->frames[i].name);
  printf("written (%d of 10) sprite name:  %s%d \n", written,framenametmp,nrframe);
  }
  else
  {
        fprintf(modeldef, "//frame %s%d %c 0 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  fprintf(modeldef, "//frame %s%d %c 1 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  printf("skipped sprite name:  %s%d \n", framename,nrframe);
  }

       }
       else
       {

         if (write==1)
  {
  fprintf(modeldef, "frame %s%d %c 0 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  printf("written (%d of 10) sprite name:  %s%d \n", written,framename,nrframe);
  }
   else if (write==2)
  {
  fprintf(modeldef, "frame %s%d %c 0 \"%s\" \n", framenametmp,nrframe,letter, mdl->frames[i].name);
  printf("written (%d of 10) sprite name:  %s%d \n", written,framenametmp,nrframe);
  }
  else
  {
        fprintf(modeldef, "//frame %s%d %c 0 \"%s\" \n", framename,nrframe,letter, mdl->frames[i].name);
  printf("skipped sprite name:  %s%d \n", framename,nrframe);
  }

       }

      fread (mdl->frames[i].verts, sizeof (struct md2_vertex_t),
	     mdl->header.num_vertices, fp);
    }

  fclose (fp);

  /* Read header */
  fread (&mdl->header, 1, sizeof (struct md2_header_t), fp);


      printf("done... \n");

  printf("--------------------------------------- \n");
  printf("set num: %d \n", setnum);
  printf("--------------------------------------- \n");
  printf("HEADER MD2 FILE: %s \n", filename);
  printf("--------------------------------------- \n");
  printf("ident: %d \n", mdl->header.ident);
  printf("version: %d \n", mdl->header.version);
  printf("--------------------------------------- \n");
  printf("skinwidth: %d \n", mdl->header.skinwidth);
  printf("skinheight: %d \n", mdl->header.skinheight);
  printf("--------------------------------------- \n");
  printf("framesize: %d \n", mdl->header.framesize);
  printf("--------------------------------------- \n");
  printf("num_skins: %d \n", mdl->header.num_skins);
  printf("num_vertices: %d \n", mdl->header.num_vertices);
  printf("num_st: %d \n", mdl->header.num_st);
  printf("num_tris: %d \n", mdl->header.num_tris);
  printf("num_glcmds: %d\n", mdl->header.num_glcmds);
  printf("num_frames: %d\n", mdl->header.num_frames);
  printf("--------------------------------------- \n");
  printf("offset_skins: %d \n", mdl->header.offset_skins);
  printf("offset_st: %d \n", mdl->header.offset_st);
  printf("offset_tris: %d \n", mdl->header.offset_tris);
  printf("offset_frames: %d \n", mdl->header.offset_frames);
  printf("offset_glcmds: %d \n", mdl->header.offset_glcmds);
  printf("offset_end: %d \n", mdl->header.offset_end);
  printf("--------------------------------------- \n");

  return 1;
}

/**
 * Free resources allocated for the model.
 */
void
FreeModel (struct md2_model_t *mdl)
{
  int i;

  if (mdl->skins)
    {
      free (mdl->skins);
      mdl->skins = NULL;
    }

  if (mdl->texcoords)
    {
      free (mdl->texcoords);
      mdl->texcoords = NULL;
    }

  if (mdl->triangles)
    {
      free (mdl->triangles);
      mdl->triangles = NULL;
    }

  if (mdl->glcmds)
    {
      free (mdl->glcmds);
      mdl->glcmds = NULL;
    }

  if (mdl->frames)
    {
      for (i = 0; i < mdl->header.num_frames; ++i)
	{
	  free (mdl->frames[i].verts);
	  mdl->frames[i].verts = NULL;
	}

      free (mdl->frames);
      mdl->frames = NULL;
    }
}


/**
 * Calculate the current frame in animation beginning at frame
 * 'start' and ending at frame 'end', given interpolation percent.
 * interp will be reseted to 0.0 if the next frame is reached.
 */
void
Animate (int start, int end, int *frame, float *interp)
{
  if ((*frame < start) || (*frame > end))
    *frame = start;

  if (*interp >= 1.0f)
    {
      /* Move to next frame */
      *interp = 0.0f;
      (*frame)++;

      if (*frame >= end)
	*frame = start;
    }
}

void
init (const char *filename)
{

  /* Load MD2 model file */
  if (!ReadMD2Model (filename, &md2file))
    exit (EXIT_FAILURE);

  //close
    fprintf(modeldef, "} \n");
    fclose(modeldef);

  FreeModel (&md2file);

}

int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr, "usage: %s <filename.md2>\n", argv[0]);
      return 0; //-1;
    }
    else
    {
      init (argv[1]);
      printf("wrote file : %s  \n", mdfilename);
    }

  return 0;
}


