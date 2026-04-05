#pragma once

struct SItem{
	std::vector<SID_T> pids;
	uint32_t asid;
	uint32_t sid;
	xstring nick;
	UID_T owner;
	int	 users;
	bool bLimit;
	bool bFolder;
	bool bPub;
	xstring intro;
	xstring	lables;
	xstring slogan;
	int collect;
	int member;
	int  fond;
	SItem():users(0), bFolder(false),bPub(true),owner(0), collect(0),member(0),fond(0) {}
	int hasParent(const SID_T &p) const{
		if(std::find(pids.begin(), pids.end(), p) != pids.end()){
			if(pids.size() > 1)
				return 2;
			else
				return 1;
		}else
			return 0;
	}

	void rmParent(const SID_T &p){
		std::vector<SID_T>::iterator pi = std::find(pids.begin(), pids.end(), p);
		pids.erase(pi);
	}
};