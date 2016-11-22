/**@<pseudoassembly.c>::**/

int labelcounter = 1;
int gofalse(int label){
      fprintf(object, "\tjz .L%d\n", label);
}

int jump(int label){
      fprintf(object, "\tjmp .L%d\n", label);
}

int mklabel(int label){
      fprintf(object, ".L%d:\n", label);
      return label;
}

//unary
int neglog(void){
}

int negint(void){
}

int negflt(void){
}
int negdbl(void){
}

//binary addition and inverse
int addlog(void){
}

int addint(void){
}

int addflt(void){
}

int adddbl(void){
}

int subint(void){
}

int subflt(void){
}

int subdbl(void){
}

//binary multiplication and inverse
int mullog(void){
}

int mulint(void){
}

int mulflt(void){
}

int muldbl(void){
}

int divint(void){
}

int divflt(void){
}

int divdbl(void){
}
