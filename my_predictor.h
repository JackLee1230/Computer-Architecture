// my_predictor.h
// This file contains a sample my_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 8 kilobytes available
// for the CBP-2 contest; it is just an example.

//This is a piecewise linear branch predictor.
class my_update : public branch_update {
public:
	unsigned int index;
	int output;
	int predict;
};

class my_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	15
#define TABLE_BITS	15
#define W_X (2^10)
#define W_Y (2^4)
#define W_Z (HISTORY_LENGTH+1)
#define T 54
//W is a three-dimensional array of integers
//Threshold T=2.14*(HISTORY_LENGTH+1)+20.5


	my_update u;
	branch_info bi;
    unsigned int history;
    bool GHR[HISTORY_LENGTH];
	unsigned char tab[1<<TABLE_BITS];
	unsigned int global_address[HISTORY_LENGTH];
    int W[W_X][W_Y][W_Z];
	
	my_predictor (void) : history(0) {
		int a,b,c;
		memset (tab, 0, sizeof (tab));
		for(int a=0;a<HISTORY_LENGTH;a++) {
			global_address[a]=0;
		}
		srand (time(NULL));
		for(a=0;a<W_X;a++) {
			for(b=0;b<W_Y;b++) {
				for(c=0;c<W_Z;c++) {
					W[a][b][c]=rand()%256 - 127;
				}
			}
		}
	}

	branch_update *predict (branch_info & b) {
		
		bi = b;


		if (b.br_flags & BR_CONDITIONAL) {
			int x,y;
			x=b.address%W_X;
			int output = W[x][0][0];
			for(int i=0;i<HISTORY_LENGTH;i++) {
				y=global_address[i]%W_Y;
				if(GHR[HISTORY_LENGTH]) {
					output = output + W[x][y][i];
				}
				else {
					output = output - W[x][y][i];
				}
			}
			if(output>=0) {
				u.direction_prediction(1);
				u.predict=1;
			}
			else { 
				u.direction_prediction(0);
				u.predict=0;
			}
			u.output = output;	
		}
	
		
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
			unsigned char *c = &tab[((my_update*)u)->index];
			if (taken) {
				if (*c < 3) (*c)++;
			} else {
				if (*c > 0) (*c)--;
			}
	
			
			int x;
			if(abs(((my_update*)u)->output)<T || ((my_update*)u)->predict!=taken) {
				x=bi.address%W_X;
				if(taken) {
					W[x][0][0] = W[x][0][0]+1;
				}
				else {
					W[x][0][0] = W[x][0][0]-1;
				}
				for(int i =0;i<HISTORY_LENGTH;i++) {
					int y = global_address[i]%W_Y;
					if(GHR[HISTORY_LENGTH]== taken) {
						W[x][y][i] = W[x][y][i]+1;
					}
					else {
						W[x][y][i] = W[x][y][i]-1;
					}
				}
			}
			
			history <<= 1;
			history |= taken;
			history &= (1<<HISTORY_LENGTH)-1;
			
			/*global address update*/
			for(int i=HISTORY_LENGTH-1;i>=1;i--) {
				global_address[i]=global_address[i-1];
			}	
			if(HISTORY_LENGTH>0)	
			global_address[0]=target;

		}
	}
};
