#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include <vector>

/************
 * Rename
 * Allocate nre rob entry at tail
 * 
 * 
 */

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/

using namespace std;

struct RMT{  //rename map table
    bool valid = 0;
    uint32_t tag=0;
};

struct ROB{
    int number = 0; // rob0,rob1 ...
    int age; // do we need valid bit
    int destination = 0;
    // no mis,exe needed
    bool ready = false;
    struct ROB *head;  // Head pointer
    struct ROB *tail;  // Tail pointer
    uint32_t pc = 0;
};

struct IQ{  //issue queue
    bool valid = 0;
    int destination_tag = 0;
    bool source1_ready = false;
    int source1_value = 0;
    int source1_tag=0;
    int source1_rob=0;
    bool source2_ready=false;
    int source2_value=0;
    int source2_tag=0;
    int source2_rob=0;
};


// struct FE{
//     bool valid=false;
//     int op_type=0;
//     int destination=0;
//     int source1=0;
//     int source1_rob=0;
//     int source2=0;
//     int source2_rob=0;

//     int age=0;
// };


struct DE{
    bool valid; // if instructions present true
    int op_type=0;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct RN{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct RR{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct DI{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct IS{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct EX{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct WB{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

struct RT{
    bool valid;
    int op_type;
    int destination = 0;
    int source1 = 0;
    int source1_rob = 0;
    int source2 = 0;
    int source2_rob = 0;

    int age = 0;
};

class superscalar{
    public:
    int head=0,tail=0; // increment whenever
    int width,iq_size,rob_size;
    uint64_t pc;
    int instructions_count=0;
    struct ROB *head_ptr; // pointer to the head of rob
    struct ROB *tail_ptr; // tail of rob
    vector <ROB> rob;
    vector <RMT> rmt;
    vector <RT> retire;
    vector <WB> writeback;
    vector <EX> execute_list;
    vector <DI> dispatch;
    vector <IS> issue_q;
    vector <RR> reg_read;
    vector <RN> rename;
    vector <DE> decode;

    superscalar(int width,int iq_size, int rob_size){
        this->width = width;
        this->iq_size = iq_size;
        this->rob_size = rob_size;
        head_ptr = &rob[0];
        tail_ptr = &rob[0];
        rob.resize(rob_size);
        writeback.resize(width*5);
        execute_list.resize(width*5);
        dispatch.resize(width);
        issue_q.resize(iq_size);
        reg_read.resize(width);
        rename.resize(width);
        decode.resize(width);
    }
    
    void decode(){
        bool bundle_present = false;
        bool rename_vacant = false;
        int rename_count=0;

        for(int i = 0;i<width;i++){
            if(decode[i].valid == 1){
                bundle_present = true;
            }
        }

        if(bundle_present){
            for(int i = 0;i < width;i++){
                if(!rename[i].valid){
                    rename_count+=1;
                }
            }
            rename_vacant = (rename_count == width);

            if (rename_vacant){
                for(int i=0;i < width;i++){
                    // if(rename[i].valid)
                    rename[i].op_type = decode[i].op_type;
                    rename[i].destination = decode[i].destination;
                    rename[i].source1 = decode[i].source1;
                    rename[i].source2 = decode[i].source2;
                    rename[i].age = decode[i].age;
                    rename[i].valid = decode[i].valid;
                    decode[i].valid = false;                    
                }
            }
            
        }
    }

    void fetch(FILE *fp){
        int decode_instructions_count=0;
        for(int i=0;i<width;i++){
            if(decode[i].valid){
                decode_instructions_count++;
            }
        }
        if(decode_instructions_count==0 && !(feof(fp))){  //more instructions in the  trace file and if DE is empty
            for(int i=0;i<width;i++){    //fetch upto width instructions
                fscanf(fp,"%llx %d %d %d %d\n",&pc,&decode[i].op_type,&decode[i].destination,&decode[i].source1,&decode[i].source2);
                decode[i].valid = true;
                instructions_count += 1;
                decode[i].age=instructions_count;


                if(feof(fp)){
                    break; // no more instructions
                }
            }
        }
    }
    
};


int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    // from the fetch function we read the tracefile

    // do{

    // }while(Advance_Cycle());


    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
        printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly

    return 0;
}
