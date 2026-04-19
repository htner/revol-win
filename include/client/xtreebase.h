#pragma	  once

#include "client/proto/IProto.h"
#include "common/nsox/selector.h"
#include "common/nsox/nuautoptr.h"
#include <map>
namespace xproto
{

		class XTreeBase : public ITreeBase
						, public XConnPoint<ITreeEvent>
		{
		public:
			XTreeBase(){	
					__firstChild = __lastChild = NULL;
					__nextSibling = __prevSibling = NULL;
					__parent	 = NULL;
					__showEvent  = true;
			}
			bool	showEvent(bool bShow)
			{
					__showEvent = bShow;
			}
			virtual bool		hasChild()
			{
					return __firstChild != NULL;
			}
			virtual ITreeBase*	parent()
			{
					return __parent;
			}
			virtual	ITreeBase*	nextSibling()
			{
					return __nextSibling;
			}
			virtual	ITreeBase*	prevSibling()
			{
					return __prevSibling;
			}
			virtual ITreeBase*  firstChild()
			{
					return  __firstChild;	
			}
			virtual ITreeBase*	lastChild()
			{
					return __lastChild;
			}
			virtual	ITreeBase*	find(LPCSTR id)
			{
					CHILDMAP::iterator itr = __childmap.find(id);
					if(itr != __childmap.end()){
							return itr->second;
					}
					return NULL;
			}
			virtual xstring		getId()
			{
					return __id;
			}
			virtual ITreeBase*	remove(LPCSTR id)
			{
					TreeType_t xx = (XTreeBase* )find(id);
					if(xx != NULL){
							__childmap.erase(id);

							TreeType_t temp = xx->__nextSibling;
							xx->__prevSibling->__nextSibling =  xx->__nextSibling;
							xx->__nextSibling->__prevSibling =	xx->__prevSibling;

							xx->__nextSibling = NULL;
							xx->__prevSibling = NULL;
							xx->__parent	  = NULL;

							if(__showEvent){
								forEachWatcher2(&ITreeEvent::onDelete, (ITreeBase*)this, (ITreeBase*)xx);
							}							
							return xx;							
					}			
					return NULL;
			}

			ITreeBase*	insert(LPCSTR id, XTreeBase* child)
			{
					TreeType_t xx = (XTreeBase* )find(id);
					if(xx = NULL){ //append to 
							if(__firstChild){
									__lastChild->__nextSibling = child;
									child->__prevSibling = __lastChild;
									__lastChild = child;
							}else{
									__firstChild = __lastChild = child;	
							}
					}else{
							XTreeBase* pp = xx->__prevSibling;
							child->__nextSibling = xx->__nextSibling;
							xx->__prevSibling = child;

							if(pp){
								pp->__nextSibling = child;
								child->__prevSibling = pp;
							}else{
								__firstChild = child;								
							}
					}
					if(__showEvent){
							__childmap[child->getId()] = child;
					}
					child->__parent = this;

					forEachWatcher2(&ITreeEvent::onAdd, (ITreeBase*)this, (ITreeBase*)xx);

					return child;
			}
			int	size(){
					return __childmap.size();	
			}
			virtual ITreeBase*  insertBefore(LPCSTR id){
						TreeType_t child = nsox::nu_create_object<XTreeBase>::create_inst();
						child->__id = id;
						return insert(id, child);
			}
			virtual ITreeBase*	appendChild(){
						
						TreeType_t child = nsox::nu_create_object<XTreeBase>::create_inst();
						return insert("", child);
			}

			virtual ITreeObject* getObject(){
					return __treeObject;
			}
		
			virtual void	setObject(ITreeObject* obj){
					__treeObject = obj;
			}
		
		protected:

			typedef nsox::nu_auto_ptr<XTreeBase> TreeType_t;
			typedef std::map<xstring,TreeType_t> CHILDMAP;

			CHILDMAP			__childmap;
			
			XTreeBase*			__parent;
			XTreeBase*			__firstChild;
			XTreeBase*			__nextSibling;
			XTreeBase*			__prevSibling;
			XTreeBase*			__lastChild;
		
			xstring				__id;
			bool				__showEvent;
			nsox::nu_auto_ptr<ITreeObject> __treeObject;
		};
}