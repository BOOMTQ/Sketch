// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// Allocate memory for a drawing state and initialise it
state *newState() {
  state *s=(state*)malloc(sizeof(state));
  s->x=0;
  s->y=0;
  s->tx=0;
  s->ty=0;
  s->start=0;
  s->end=false;
  s->data=0;
  s->tool=LINE;
  //TO DO
  return s; // this is a placeholder only
}

// Release all memory associated with the drawing state
void freeState(state *s) {
  free(s);//TO DO
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
  //TO DO
  int num=b;
  int fir=(num>>7)&1,sec=(num>>6)&1;
  if(fir&&sec)return DATA;
  if(fir&&!sec)return TOOL;
  if(!fir&&sec)return DY;
  return DX; // this is a placeholder only
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
  int sig=b>>5&1;
  if(sig){
    int res=0;
    for(int i=0,j=1;i<5;i++,j*=2){
      res+=(b>>i&1)*j;
    }
    return  -1*(32-res);
  }
    int res=0;
    for(int i=0,j=1;i<5;i++,j*=2){
      res+=(b>>i&1)*j;
    }
    return  res;
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  
  int pos=getOpcode(op);
  //printf("%X %d\n",op,pos);
  if(pos==TOOL){
	int rest=op-128;
	if(!rest){
		s->tool=NONE;
	}
	else if(rest==1){
		s->tool=LINE;
	}
	else if(rest==2){
		s->tool=BLOCK;
	}
	else if(rest==3){
		//printf("%d\n",s->data);       
		colour(d, s->data);
	}
	else if(rest==4){
		s->tx=s->data;
	}
	else if(rest==5){
		s->ty=s->data;
	}
	else if(rest==6){
		show(d);
	}
	else if(rest==7){
		pause(d,s->data);
	}
	else if(rest==8){
		s->end=true;
	}
	s->data=0;
  }
  else if(pos==DX){
    int res=getOperand(op);
    s->tx+=res;
    
  }
  else if(pos==DY){
	int res=getOperand(op);
    s->ty+=res;
	if(s->tool==BLOCK){
		block(d,s->x,s->y,abs(s->x-(s->tx)),abs(s->y-(s->ty)));
	}
    else if(s->tool==LINE)line(d,s->x,s->y,s->tx,s->ty);
    s->x=s->tx,s->y=s->ty;
   
  }
  else if(pos==DATA){
	s->data<<=6;
	s->data+=op-192;
  }
  //printf("%d\n\n",s->data);
  //printf("%d %d %d %d\n",s->x,s->y,s->tx,s->ty);
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {
    //TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    //NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return (pressedKey == 27);
    //NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state *s = (state*) data;
    //NOTE: TO GET THE FILENAME... char *filename = getName(d);
    //NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING STATE APART FROM
    //      THE 'START' FIELD AFTER CLOSING THE FILE
  if (data == NULL) return (pressedKey == 27);
  FILE *fp = NULL;
  char *filename = getName(d);
  int ch=0,testcnt=0;
  fp = fopen(filename, "r");
  state *s = (state*) data;
  //printf("s->start=%d\n",s->start);
  while(s->end==true&&testcnt!=s->start&&((ch=fgetc(fp)) != EOF) ){
	  testcnt++;
  }
  s->end=false;
  while( (ch=fgetc(fp)) != EOF ){
	  //printf("%d\n",ch);
	  obey(d,s,ch);
	  testcnt++;
	  if(s->end==true){
		  s->start=testcnt;
		  break;
	  }
	  //printf("%d %d\n\n",ch,s->data);
  }
  show(d);
  fclose(fp);
  s->x=0;
  s->y=0;
  s->tx=0;
  s->ty=0;
  s->data=0;
  s->tool=LINE;
  return (pressedKey == 27);

}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
