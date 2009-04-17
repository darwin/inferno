#define NUMRAYPASSES 700
#define NUMPASSES (NUMRAYPASSES+6)*((NUMLIGHTS_NOHACK+1)*(NUMSAMPLES_NOHACK))


#define SHADING_SIMPLE       1
#define SHADING_FLAT         2
#define SHADING_SMOOTH       3
#define SHADING_CUSTOM1      4

// choose shading model
#define SHADING SHADING_SIMPLE

// state machine
//! MAX 16 states (4bit)
#define STATE_DONE             0
#define STATE_NONE             1
#define STATE_TRAVERSING       2
#define STATE_INTERSECTING     3
#define STATE_SHADING          4
#define STATE_DISPATCHING      5

#define DEFINE_STATE_TABLE char* state_table[] = { "DONE", "NONE", "TRAVERSING", "INTERSECTING", "SHADING", "DISPATCHING" }

#define SRCSTREAM(name) ((!flip_##name)?(a_##name):(b_##name))
#define DSTSTREAM(name) ((flip_##name)?(a_##name):(b_##name))

#define FLIP(name) ((flip_##name)?(flip_##name=0):(flip_##name=1))

#define STATECODE(oldcode, newcode) ((oldcode&0x0000000F)|(newcode<<4)&0x000000F0)
