#include <iostream>
#include <cstring>
#include <cstdio>
//面向硬件编程
enum INSTRUCTION{
    LUI,      //U
    AUIPC,    //U  
    JAL,      //J
    JALR,     //I
    BEQ,      //B
    BNE,      //B
    BLT,      //B
    BGE,      //B
    BLTU,     //B
    BGEU,     //B
    LB,       //I
    LH,       //I
    LW,       //I
    LBU,      //I
    LHU,      //I
    SB,       //S
    SH,       //S
    SW,       //S
    ADDI,     //I
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,      //R
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND
};
const char* name[] = {"LUI","AUIPC","JAL","JALR","BEQ","BNE","BLT","BGE","BLTU","BGEU","LB","LH","LW","LBU","LHU","SB","SH","SW","ADDI","SLTI","SLTIU","XORI","ORI","ANDI","SLLI","SRLI","SRAI","ADD","SUB","SLL","SLT","SLTU","XOR","SRL","SRA","OR","AND"};
const char* REGI_name[] = {"zero","ra","sp","qp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
enum INSTRUCTION_TYPE{_U,_J,_B,_I,_S,_R};
struct error{
    unsigned int line,num;
    error(unsigned int l,unsigned int n):line(l),num(n){}
    void printerror(){
        std::cerr<<"ERROR in line "<<line<<"!\tnum = "<<num<<std::endl;
    }
};
struct move{};
struct end{
    int num;
    end(int n=0):num(n){}
};
inline bool is_num(char c){
    if(c>='0'&&c<='9') return true;
    if(c>='A'&&c<='F') return true;
    if(c=='@') throw move();
    if(c==EOF) throw end();
    return false;
}


class IF_IDs{
public:
    bool NOP=true;
    uint32_t PC=0;//执行这条指令的PC
    uint32_t raw_instruction=0;
};

class ID_EXs{//9个元素
public:
    bool NOP=true;//EX是否为空指令
    uint32_t PC=0,NPC=0;//执行这条指令的PC,预测PC
    INSTRUCTION instruction=LUI;
    INSTRUCTION_TYPE type=_J;
    uint32_t imm=0,rs1=0,rs2=0,rd=0;
};

class EX_MEMs{//8个元素
public:
    bool NOP=true;//MEM是否为空指令
    uint32_t PC=0;//PC,预测PC
    INSTRUCTION instruction=LUI;
    INSTRUCTION_TYPE type=_J;

    uint32_t MEM_index=0;
    uint32_t to_MEM=0;

    uint32_t to_WB=0;
    uint32_t WB_index=0;
};

class MEM_WBs{
public:
    bool NOP=true;//WB是否为空指令
    INSTRUCTION instruction;
    INSTRUCTION_TYPE type=_J;
    // uint32_t PC=0;
    uint16_t WB_index=0;
    uint32_t to_WB=0;
};

class BroadCast{
public:
    bool NOP=true;
    INSTRUCTION instruction;
    uint32_t rd=0;//1~31
    uint32_t value=0;
};

INSTRUCTION decoder(uint32_t num){//取raw指令,返回指令
    INSTRUCTION instruction;
    if(num%4 != 0x3) throw error(__LINE__,num);
    unsigned int sub_num1 = (num>>12)%8;
    if( (num>>2)%2 == 0x1) {//                      111
        if((num>>3)%2 == 0x1){//                   1111
            instruction = JAL;
        } else {//                                 0111
            if((num>>4)%2== 0x1){//               10111
                if((num>>5)%2 == 0x1){//         010111
                    instruction = LUI;
                }else {//                        110111
                    instruction = AUIPC;
                }
            } else{//                             00111
                instruction = JALR;
            }
        }
    }else{//                                        011
        if((num>>3)%2 == 0x1) throw error(__LINE__,num);//     1011
        if((num>>4)%2 == 0x1){//                  10011
            if((num>>6)%2 == 0x1) throw error(__LINE__,num);
            unsigned int sub_num2 = (num>>30);
            if((num>>5)%2 == 0x1){//            0110011
                switch (sub_num1){
                    case 0x0:
                        if(sub_num2==0x1) instruction = SUB;
                        else instruction = ADD;
                    break;
                    case 0x1: instruction = SLL;  break;
                    case 0x2: instruction = SLT;  break;
                    case 0x3: instruction = SLTU; break;
                    case 0x4: instruction = XOR;  break;
                    case 0x5:
                        if(sub_num2==0x1) instruction = SRA;
                        else instruction = SRL;
                    break;
                    case 0x6: instruction = OR;   break;
                    case 0x7: instruction = AND;  break;
                    default: throw error(__LINE__,num);       break;
                }
            }else{//                            0010011
                switch (sub_num1){
                    case 0x0: instruction = ADDI; break;
                    case 0x1: instruction = SLLI; break;
                    case 0x2: instruction = SLTI; break;
                    case 0x3: instruction = SLTIU;break;
                    case 0x4: instruction = XORI; break;
                    case 0x5:
                        if(sub_num2==0x1) instruction = SRAI;
                        else instruction = SRLI;
                    break;
                    case 0x6: instruction = ORI;  break;
                    case 0x7: instruction = ANDI; break;
                    default: throw error(__LINE__,num);       break;
                }
            }
        }else{//                                  00011
            if((num>>5)%2 == 0x1){//             100011
                if((num>>6)%2 == 0x1){//            1100011
                    switch (sub_num1){//             1100011
                        case 0x0: instruction = BEQ;  break;
                        case 0x1: instruction = BNE;  break;
                        case 0x4: instruction = BLT;  break;
                        case 0x5: instruction = BGE;  break;
                        case 0x6: instruction = BLTU; break;
                        case 0x7: instruction = BGEU; break;
                        default:  throw error(__LINE__,num);      break;
                    }
                }else{//                            0100011
                    switch (sub_num1){
                        case 0x0: instruction = SB;   break;
                        case 0x1: instruction = SH;   break;
                        case 0x2: instruction = SW;   break;
                        default:  throw error(__LINE__,num);      break;
                    }
                }
            }else{//                             000011
                if((num>>6)%2 == 0x1) throw error(__LINE__,num);
                switch (sub_num1){//             0000011
                    case 0x0: instruction = LB;   break;
                    case 0x1: instruction = LH;   break;
                    case 0x2: instruction = LW;   break;
                    case 0x4: instruction = LBU;  break;
                    case 0x5: instruction = LHU;  break;
                    default:  throw error(__LINE__,num);      break;
                }
            }//end 000011
        }//end 00011
    }//end 011
    return instruction;
}
INSTRUCTION_TYPE instruction_type(INSTRUCTION instr){
    if(instr>=ADD) return _R;
    if(instr<=AUIPC) return _U;
    if(instr==JAL) return _J;
    if(instr>=BEQ&&instr<=BGEU) return _B;
    if(instr>=SB&&instr<=SW) return _S;
    return _I;
}

class CPU{
private:
    uint32_t MEMORY[1<<16]={0};
    uint32_t REGI[32]={0};//寄存器
    uint32_t PC=0;//系统PC
    uint64_t tick=0;//时钟
    IF_IDs IF_ID;
    ID_EXs ID_EX;
    EX_MEMs EX_MEM;
    MEM_WBs MEM_WB;

    void lui(){//EX阶段运行
        EX_MEM.to_WB     = ID_EX.imm;
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void auipc(){
        EX_MEM.to_WB     = ID_EX.imm+ID_EX.rd;
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void jal(){
        EX_MEM.to_WB     = ID_EX.PC+4;
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.PC        = ID_EX.PC+ID_EX.imm;
    }
    void beq(){
        if(ID_EX.rs1==ID_EX.rs2){
             EX_MEM.PC  = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void bne(){
        if(ID_EX.rs1!=ID_EX.rs2) {
            EX_MEM.PC   = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void blt(){
        if((int)ID_EX.rs1< (int)ID_EX.rs2) {
            EX_MEM.PC   = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void bge(){
        if((int)ID_EX.rs1>=(int)ID_EX.rs2) {
            EX_MEM.PC   = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void bltu(){
        if(ID_EX.rs1< ID_EX.rs2) {
            EX_MEM.PC   = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void bgeu(){
        if(ID_EX.rs1>=ID_EX.rs2) {
            EX_MEM.PC   = ID_EX.PC+ID_EX.imm;
        }
        else EX_MEM.PC  = ID_EX.PC+4;
    }
    void jalr(){
        EX_MEM.to_WB     = ID_EX.PC+4;
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.PC        = (ID_EX.imm+ID_EX.rs1)&(-2u);
    }
    void lb(){//在MEM阶段解决
        char ch;
        memcpy(&ch,(char*)MEMORY+EX_MEM.MEM_index,sizeof(char));
        MEM_WB.to_WB     = (uint32_t)ch;
        MEM_WB.WB_index  = EX_MEM.WB_index;
    }
    void lh(){//在MEM阶段解决
        short ch;
        memcpy(&ch,(char*)MEMORY+EX_MEM.MEM_index,sizeof(short));
        EX_MEM.to_WB     = (uint32_t)ch;
    }
    void lw(){//在MEM阶段解决
        uint32_t ch;
        memcpy(&ch,(char*)MEMORY+EX_MEM.MEM_index,sizeof(unsigned int));
        EX_MEM.to_WB     = (uint32_t)ch;
    }
    void lbu(){//在MEM阶段解决
        unsigned char ch;
        memcpy(&ch,(char*)MEMORY+EX_MEM.MEM_index,sizeof(unsigned char));
        EX_MEM.to_WB     = (uint32_t)ch;
    }
    void lhu(){//在MEM阶段解决
        unsigned short ch;
        memcpy(&ch,(char*)MEMORY+EX_MEM.MEM_index,sizeof(unsigned short));
        EX_MEM.to_WB     = (uint32_t)ch;
    }
    void addi(){//EX阶段解决
        if(ID_EX.rd==10&&ID_EX.imm==0xFF)//结束标记
            throw end(REGI[ID_EX.rd]&0xFFUL);
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1+ID_EX.imm;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void slti(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ((int)ID_EX.rs1<(int)ID_EX.imm);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void sltiu(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (ID_EX.rs1<ID_EX.imm);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void xori(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (ID_EX.rs1^ID_EX.imm);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void ori(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (ID_EX.rs1|ID_EX.imm);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void andi(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (ID_EX.rs1&ID_EX.imm);
        EX_MEM.PC        = ID_EX.PC+4;
    }

    void slli(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1<<(ID_EX.imm&31UL);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void srli(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1>>(ID_EX.imm&31UL);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void srai(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (int) ID_EX.rs1>>(ID_EX.imm&31UL);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void sb(){//在MEM阶段解决
        char ch=(char) EX_MEM.to_MEM;
        memcpy((char*)MEMORY+EX_MEM.MEM_index,&ch,sizeof(char));
    }
    void sh(){
        short ch=(unsigned short) EX_MEM.to_MEM;
        memcpy((char*)MEMORY+EX_MEM.MEM_index,&ch,sizeof(unsigned short));
    }
    void sw(){
        uint32_t ch=EX_MEM.to_MEM;
        memcpy((char*)MEMORY+EX_MEM.MEM_index,&ch,sizeof(unsigned int));
    }
    void add(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1+ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void sub(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1-ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }

    void sll(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1<<(ID_EX.rs2&31);
        EX_MEM.PC        = ID_EX.PC+4;
    }//逻辑左移

    void slt(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (int)ID_EX.rs1<(int)ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void sltu(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1<ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }

    void _xor(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1^ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void srl(){//逻辑右移(空位补0)
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1>>(ID_EX.rs2&31UL);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void sra(){//算数右移(空位用最高位填充)
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = (int)(ID_EX.rs1)>>(ID_EX.rs2&31UL);
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void _or(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1|ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    void _and(){
        EX_MEM.WB_index  = ID_EX.rd;
        EX_MEM.to_WB     = ID_EX.rs1&ID_EX.rs2;
        EX_MEM.PC        = ID_EX.PC+4;
    }
    uint32_t predictor(const ID_EXs& ID_EX){//预测NPC
        return ID_EX.PC+4UL;
    }
    void read(uint32_t& num){
        char ch=getchar();uint32_t x=0;
        while(!is_num(ch)){ch=getchar();}
        while(is_num(ch)){
            ch = (ch<'A')?(ch-'0'):(ch-'A'+10);
            x=(x<<4)+ch;ch=getchar();
        }
        num = x;
    }
    uint32_t read(){//把32位码转换成int
        char ch[8];
        ch[1] = getchar();
        while(!is_num(ch[1])){ch[1]=getchar();}
                        ch[0] = getchar();getchar();
        ch[3] = getchar();ch[2] = getchar();getchar();
        ch[5] = getchar();ch[4] = getchar();getchar();
        ch[7] = getchar();ch[6] = getchar();

        unsigned int ans=0;
        for(int i=0;i<8;i++){
            if(ch[i]<'A') ch[i] = ch[i]-'0';
            else ch[i] = ch[i]-'A'+10;
            ans |= ((unsigned int)(ch[i]) << (i<<2) );
        }
        return ans;
    }
    
    void IF(){
        IF_ID.raw_instruction=MEMORY[PC/4];
        // printf("IF %s  PC = 0x%x\n",name[decoder(IF_ID.raw_instruction)],PC);
        IF_ID.PC=this->PC;//一开始的PC
        PC+=4;
        IF_ID.NOP=false;
    }
    void check_broadcast(uint32_t& rs){
        WB();
        MEM();
        WB();
        rs = REGI[rs];
        return;
//====================================不冒险，直接停=========================================
        const uint32_t prev_rd1 = (EX_MEM.type!=_S && EX_MEM.type!=_B && !EX_MEM.NOP)?EX_MEM.WB_index:320;//还没MEM 修改WB的寄存器
        const uint32_t prev_rd2 = (MEM_WB.type!=_S && MEM_WB.type!=_B && !MEM_WB.NOP)?MEM_WB.WB_index:320;//还没WB  修改WB的寄存器
        uint32_t& prev_value1 = EX_MEM.to_WB;
        uint32_t& prev_value2 = MEM_WB.to_WB;
        if(rs==prev_rd1){
            // if(EX_MEM.instruction==LB||EX_MEM.instruction==LH||
            // EX_MEM.instruction==LBU||EX_MEM.instruction==LHU||EX_MEM.instruction==LW){

                puts("**********MEM************");
                WB();
                MEM();
                WB();
                puts("**********MEM************");
            // }
            // rs=prev_value1;
            rs = REGI[rs];
            puts("-----prev_value1-----");
        }
        else if(rs==prev_rd2){
                puts("**********MEM************");
                WB();
                puts("**********MEM************");
            rs = REGI[rs];
            puts("-----prev_value2-----");
        }
        else rs=REGI[rs];
    }
    void ID(){
        if(IF_ID.NOP) return;
        else IF_ID.NOP=true;
        const uint32_t& num=IF_ID.raw_instruction;//寄存器取指令
        ID_EX.instruction=decoder(IF_ID.raw_instruction);//1

        ID_EX.type=instruction_type(ID_EX.instruction);//2
        ID_EX.PC=IF_ID.PC;//3
        ID_EX.imm=0;//4
        ID_EX.rd=ID_EX.rs1=ID_EX.rs2=0;//5 6 7

        switch(ID_EX.type){//Fetch Register
            case _U:
                ID_EX.rd = (num>>7)%32;
                ID_EX.imm = (num & 0xFFFFF000UL);//不用移位
            break;

            case _J:
                ID_EX.rd = (num>>7)%32;
                if ((num >> 31u) == 1u)
                    ID_EX.imm |= 0xFFF00000UL;
                ID_EX.imm |= num & 0x000ff000UL;
                ID_EX.imm |= ((num >> 20u) & 1u) << 11u;
                ID_EX.imm |= ((num >> 21u) & 1023u) << 1u;
            break;

            case _B:
                ID_EX.rs1 = (num>>15)%32;
                ID_EX.rs2 = (num>>20)%32;
                check_broadcast(ID_EX.rs1);
                check_broadcast(ID_EX.rs2);
                if ((num >> 31u) == 1u)
                    ID_EX.imm |= 0xfffff000;
                ID_EX.imm |= ((num >> 7u) & 1u) << 11u;
                ID_EX.imm |= ((num >> 25u) & 63u) << 5u;
                ID_EX.imm |= ((num >> 8u) & 15u) << 1u;
            break;

            case _I:
                ID_EX.rs1 = (num>>15)%32;
                ID_EX.rd = (num>>7)%32;
                check_broadcast(ID_EX.rs1);
                if ((num >> 31u) == 1u)
                    ID_EX.imm |= 0xfffff800;
                ID_EX.imm |= (num >> 20u) & 2047u;
            break;

            case _S:
                ID_EX.rs1 = (num>>15)%32;
                ID_EX.rs2 = (num>>20)%32;
                check_broadcast(ID_EX.rs1);
                check_broadcast(ID_EX.rs2);

                if ((num >> 31u) == 0x1u)
                    ID_EX.imm |= 0xfffff800;
                ID_EX.imm |= ((num >> 25u) & 63u) << 5u;
                ID_EX.imm |= ((num >> 8u) & 15u) << 1u;
                ID_EX.imm |= (num >> 7u) & 1u;
            break;

            case _R:
                ID_EX.rs1 = (num>>15)%32;
                ID_EX.rs2 = (num>>20)%32;
                check_broadcast(ID_EX.rs1);
                check_broadcast(ID_EX.rs2);
                ID_EX.rd  = (num>>7)%32;
            break;
        }//判断type
        // printf("ID %s  PC = 0x%x  rs1 = %d  rs2 = %d\n",name[ID_EX.instruction],ID_EX.PC,ID_EX.rs1,ID_EX.rs2);
        ID_EX.NPC = predictor(ID_EX);//分支预测8
        ID_EX.NOP = false;//9
    }

    void EX(){
        if(ID_EX.NOP) return;//是否为空指令
        else ID_EX.NOP=true;//执行完就把指令清空

        // printf("EX %s  ",name[ID_EX.instruction]);
        switch(ID_EX.instruction){
            case LUI: lui(); break;
            case AUIPC: auipc(); break;
            case JAL: jal(); break;
            case BEQ: beq(); break;
            case BNE: bne(); break;
            case BLT: blt(); break;
            case BGE: bge(); break;
            case BLTU: bltu(); break;
            case BGEU: bgeu(); break;
            case JALR: jalr(); break;
            case LB:  case LH: case LW: case LBU: case LHU: 
            EX_MEM.MEM_index  = ID_EX.rs1+ID_EX.imm;//1 to_WB4
            // printf("MEM_index(0x%x) = rs1(%d) + imm(%d)",EX_MEM.MEM_index,ID_EX.rs1,ID_EX.imm);
            EX_MEM.WB_index   = ID_EX.rd;//2
            EX_MEM.PC         = ID_EX.PC+4;//3
            break;
            case ADDI: addi(); break;//
            case SLTI: slti(); break;
            case SLTIU: sltiu(); break;
            case XORI: xori(); break;
            case ORI: ori(); break;
            case ANDI: andi(); break;
            case SLLI: slli(); break;
            case SRLI: srli(); break;
            case SRAI: srai(); break;
            case SB: case SH: case SW: 
            EX_MEM.to_MEM    = ID_EX.rs2;//5
            EX_MEM.MEM_index = ID_EX.rs1+(int)ID_EX.imm; 
            // printf("MEM_index(0x%x) = rs1(%d) + imm(%d)",EX_MEM.MEM_index,ID_EX.rs1,ID_EX.imm);
            EX_MEM.PC        = ID_EX.PC+4;
            break;
            case ADD: add(); break;
            case SUB: sub(); break;
            case SLL: sll(); break;
            case SLT: slt(); break;
            case SLTU: sltu(); break;
            case XOR: _xor(); break;
            case SRL: srl(); break;
            case SRA: sra(); break;
            case OR: _or(); break;
            case AND: _and(); break;
        }
        // printf("PC = 0x%x  ",ID_EX.PC);
        // if(ID_EX.type!=_S&&ID_EX.type!=_B)
        //     printf("%d(%s)  imm = %d  ",EX_MEM.to_WB,REGI_name[EX_MEM.WB_index],ID_EX.imm);
        
        if(ID_EX.NPC!=EX_MEM.PC){
            // puts("jump!!");
            IF_ID.NOP=true;//停止ID
            PC=EX_MEM.PC;
        }//如果预测值不一样，那么暂停流水线
        // else puts("");
        EX_MEM.instruction=ID_EX.instruction;//6
        EX_MEM.type=ID_EX.type;//7
        EX_MEM.NOP=false;//8
    }

    void MEM(){

        if(EX_MEM.NOP) return;
        else EX_MEM.NOP=true;

        // printf("MEM %s  ",name[EX_MEM.instruction]);
        if(EX_MEM.type == _I||EX_MEM.type == _S){//如果是I or S就执行
            switch(EX_MEM.instruction){
                case LB: lb();break;
                case LH: lh();break;
                case LW: lw();break;
                case LBU: lbu();break;
                case LHU: lhu();break;
                case SB: sb();break;
                case SH: sh();break;
                case SW: sw();break;
            }
            // printf("MEM_index = 0x%x  To_MEM = 0x%x  To_WB = 0x%x\n",EX_MEM.MEM_index,EX_MEM.to_MEM,EX_MEM.to_WB);
        }
        // else puts("");
        MEM_WB.type=EX_MEM.type;
        MEM_WB.to_WB=EX_MEM.to_WB;
        MEM_WB.WB_index=EX_MEM.WB_index;
        MEM_WB.instruction=EX_MEM.instruction;
        MEM_WB.NOP=false;
    }

    void WB(){
        if(MEM_WB.NOP) return;
        else MEM_WB.NOP=true;

        if(MEM_WB.type==_S||MEM_WB.type==_B){
            // printf("WB %s\n",name[MEM_WB.instruction]);
            return;
        }
        REGI[MEM_WB.WB_index]=MEM_WB.to_WB;
        REGI[0]=0;
        // printf("WB %s   %d(%s)\n",name[MEM_WB.instruction],MEM_WB.to_WB,REGI_name[MEM_WB.WB_index]);
    }
    void copy_into_MEM(){
        uint32_t index=0UL;
        flag1:
        try{
            while (true){
                MEMORY[index/4]=read();
                index+=4;
            }
        }
        catch(move e){
            read(index);
            // printf("move to 0x%x\n",index);
            goto flag1;
        }
        catch(end e){
            // puts("successfully read!");
            index=0;
        }
        catch(error e){
            e.printerror();
            goto flag1;
        }
    }
    public:
    void run(){
        // puts("begin");
        this->copy_into_MEM();
        PC=0;
        flag2:
        try{
            while(1){
                WB();
                MEM();
                EX();
                try{ID();}catch(...){}
                IF();
                // puts("========================");
            }
        }
        catch(error e){
            e.printerror();
            printf("PC = 0x%x\n",PC);
        }
        catch(end e){
            printf("%d\n",e.num);
        }
    }

};

int main(){
    // freopen(".\\testcases\\hanoi.data","r",stdin);
    CPU my_CPU;
    my_CPU.run();
    return 0;
}