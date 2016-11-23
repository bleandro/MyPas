/**@<pseudoassembly.c>::**/
#include <stdio.h>
#include <mypas.h>


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
int lmovel(char const *variable){   //NOSSO É INVERSNO, PQ? NUM SEI, PQ DEUS QUIS
  fprintf(object, "\tmovl %%eax, %s\n", variable);
  return 0;
}
int lmoveq(char const *variable){
  fprintf(object, "\tmovq %%rax, %s\n", variable);
  return 0;
}

int rmovel(char const *variable){
  fprintf(object, "\tpushl %%eax\n\tmovl %s, %%eax\n", variable);
  return 0;
}
int rmoveq(char const *variable){
  fprintf(object, "\tpushl %%eax\n\tmovq %s, %%rax\n", variable);
  return 0;
}

//unary
int neglog(void){
}
int negint(void){
  /*  negl %eax, (%esp)
      popl %eax   */
  fprintf(object, "\tnegl %%eax, (%%esp)\n\tpopl %%eax\n");
  return 0; //verificar o que tem que retornar
}
int negflt(void){
  /*  movss  (%esp), %xmm1    //verify
      movss  %eax, %xmm0
      negss  %xmm1, %xmm0
      movss  %xmm0, %eax
      addl   $4, %esp         //pop   */
  fprintf(object, "\tmovss (%%esp), %%xmm1\n");
  fprintf(object, "\tmovss %%eax, %%xmm0\n");
  fprintf(object, "\tnegss %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovss %%xmm0, %%eax\n");
  fprintf(object, "\taddl $4, %%esp\n");
  return 0;
}
int negdbl(void){
  /*  movsd (%rsp), %xmm1
      movsd %rax, %xmm0
      negsd %xmm1, %xmm0
      movsd %xmm0, %rax
      addq  $8, $rsp        */
  fprintf(object, "\tmovsd (%%rsp), %%xmm1\n");
  fprintf(object, "\tmovsd %%rax, %%xmm0\n");
  fprintf(object, "\tnegsd %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovsd %%xmm0, %%rax\n");
  fprintf(object, "\taddq $8, $rsp\n");
  return 0;
}

//binary addition and inverse
int addlog(void){
}
int addint(void){
  /*  addl  %eax, (%esp)
      popl  %eax                */
  fprintf(object, "\taddl %%eax, (%%esp)\n\tpopl %%eax\n");
  return 0; //verificar o que tem que retornar
}
int addflt(void){
  /*  movss  (%esp), %xmm1    //verify
      movss  %eax, %xmm0
      addss  %xmm1, %xmm0
      movss  %xmm0, %eax
      addl   $4, %esp         //pop   */
  fprintf(object, "\tmovss (%%esp), %%xmm1\n");
  fprintf(object, "\tmovss %%eax, %%xmm0\n");
  fprintf(object, "\taddss %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovss %%xmm0, %%eax\n");
  fprintf(object, "\taddl $4, %%esp\n");
  return 0;
}
int adddbl(void){
  /*  movsd (%rsp), %xmm1
      movsd %rax, %xmm0
      addsd %xmm1, %xmm0
      movsd %xmm0, %rax
      addq  $8, $rsp        */
  fprintf(object, "\tmovsd (%%rsp), %%xmm1\n");
  fprintf(object, "\tmovsd %%rax, %%xmm0\n");
  fprintf(object, "\taddsd %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovsd %%xmm0, %%rax\n");
  fprintf(object, "\taddq $8, $rsp\n");
  return 0;
}

int subint(void){
  /*  subl  %eax, (%esp)
      popl  %eax                */
  fprintf(object, "\tsubl %%eax, (%%esp)\n\tpopl %%eax\n");
  return 0;
}
int subflt(void){
  /*  movss  (%esp), %xmm0
      movss  %eax, %xmm1
      subss  %xmm1, %xmm0
      movss  %xmm0, %eax
      addl   $4, %esp         //pop   */
  fprintf(object, "\tmovss (%%esp), %%xmm0\n");
  fprintf(object, "\tmovss %%eax, %%xmm1\n");
  fprintf(object, "\tsubss %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovss %%xmm0, %%eax\n");
  fprintf(object, "\taddl $4, %%esp\n");
  return 0;
}
int subdbl(void){
  /*  movsd (%rsp), %xmm0
      movsd %rax, %xmm1
      subsd %xmm1, %xmm0
      movsd %xmm0, %rax
      addq  $8, $rsp        */
  fprintf(object, "\tmovsd (%%rsp), %%xmm0\n");
  fprintf(object, "\tmovsd %%rax, %%xmm1\n");
  fprintf(object, "\tsubsd %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovsd %%xmm0, %%rax\n");
  fprintf(object, "\taddq $8, $rsp\n");
  return 0;
}

//binary multiplication and inverse
int mullog(void){
}
int mulint(void){
  /*  mull  %eax, (%esp)
      popl  %eax                */
  fprintf(object, "\tmull %%eax, (%%esp)\n\tpopl %%eax\n");
  return 0;
}
int mulflt(void){
  /*  movss  (%esp), %xmm1
      movss  %eax, %xmm0
      mulss  %xmm1, %xmm0
      movss  %xmm0, %eax
      addl   $4, %esp         //pop   */
  fprintf(object, "\tmovss (%%esp), %%xmm1\n");
  fprintf(object, "\tmovss %%eax, %%xmm0\n");
  fprintf(object, "\tmulss %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovss %%xmm0, %%eax\n");
  fprintf(object, "\taddl $4, %%esp\n");
  return 0;
}
int muldbl(void){
  /*  movsd (%rsp), %xmm1
      movsd %rax, %xmm0
      mulsd %xmm1, %xmm0
      movsd %xmm0, %rax
      addq  $8, $rsp        */
  fprintf(object, "\tmovsd (%%rsp), %%xmm1\n");
  fprintf(object, "\tmovsd %%rax, %%xmm0\n");
  fprintf(object, "\tmulsd %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovsd %%xmm0, %%rax\n");
  fprintf(object, "\taddq $8, $rsp\n");
  return 0;
}

int divint(void){
  /*  divl  %eax, (%esp)
      popl  %eax                */
  fprintf(object, "\tdivl %%eax, (%%esp)\n\tpopl %%eax\n");
  return 0;
}
int divflt(void){
  /*  movss  (%esp), %xmm0
      movss  %eax, %xmm1
      divss  %xmm1, %xmm0
      movss  %xmm0, %eax
      addl   $4, %esp         //pop   */
  fprintf(object, "\tmovss (%%esp), %%xmm0\n");
  fprintf(object, "\tmovss %%eax, %%xmm1\n");
  fprintf(object, "\tdivss %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovss %%xmm0, %%eax\n");
  fprintf(object, "\taddl $4, %%esp\n");
  return 0;
}
int divdbl(void){
  /*  movsd (%rsp), %xmm0
      movsd %rax, %xmm1
      divsd %xmm1, %xmm0
      movsd %xmm0, %rax
      addq  $8, $rsp        */
  fprintf(object, "\tmovsd (%%rsp), %%xmm0\n");
  fprintf(object, "\tmovsd %%rax, %%xmm1\n");
  fprintf(object, "\tdivsd %%xmm1, %%xmm0\n");
  fprintf(object, "\tmovsd %%xmm0, %%rax\n");
  fprintf(object, "\taddq $8, $rsp\n");
  return 0;
}

//cc program -S
