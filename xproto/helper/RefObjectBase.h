#ifndef REFOBJECTBASE_H_
#define REFOBJECTBASE_H_

template<class Inter> class RefWrap{
	int ref;
	Inter* inter;
public:
	RefWrap(){ref = 0; inter = NULL;}
	RefWrap(Inter *i):inter(i){ref = 1;}
	Inter * addRef(){
		ref++;
		return inter;
	}
	int decRef(){
		return --ref;
	}
	Inter *get(){
		return inter;
	}
};
#endif /*REFOBJECTBASE_H_*/
