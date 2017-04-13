#include "function.h"
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <vector>
#include <assert.h>
#include <errno.h>

using namespace std;

class Node;
Node* getNewNode(Node*);
//void printPrsnt();
void printIdNode();

static int * stateNum = NULL;
static void(*GlbOutput)() = NULL;
static bool(*IsOutput)() = NULL;

static Node *prjNode = NULL;
static Node *changedNode = NULL;
static pthread_t manager=0;
//static pthread_mutex_t turnMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t turnCond = PTHREAD_COND_INITIALIZER;

static vector<$$Element*> $$prsnt = vector<$$Element*>();

volatile static pthread_t turn = 0;//一个全局变量,用于在线程调度时判断轮到哪一类线程执行
static map<pthread_t, Node*> threadIDToNode;//全局映射表,线程到存储该线程节点的映射
static bool isCopied = false;//代表该结点中的成员被copied到了另一个节点,设置为true的时候析构函数不会删除动态对象
static vector<Node*> keepChilds;
static bool *pFalse=NULL;//标志当前路径是否为false
static Node* threadRoot = NULL;

static pthread_mutex_t pStackMutex = PTHREAD_MUTEX_INITIALIZER;

enum NODELENGTH//结点当前的区间长度
{
	MORE,//长度大于等于1
	EMPTY,//区间长度为0
	NONE//长度由其它结点决定
};

enum NODETYPE
{
	AND,
	PAL,
	PRJ,
	KEEP,
	ALW,
	LEAF,
	ROOT
};

class Node{
public:
	//Node(){}
	Node(pthread_t handle, Node * fatherNode, vector<Node*> vec, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit) : thread(handle), father(fatherNode), childs(vec), len(MORE), hasEmpty(false), mutex(mutex), cond(cond), isInit(isInit)
	{
		map<pthread_t, Node*>::value_type ele = make_pair(thread, this);
		assert(threadIDToNode.insert(ele).second);
		for (size_t i = 0; i < childs.size(); ++i)
			childs[i]->setFatherNode(this);
	}
	virtual ~Node()
	{
		if (!isCopied)
		{
			father = NULL;
			for (vector<Node *>::iterator it = childs.begin(); it != childs.end(); ++it)
			{
				delete *it;
				*it = NULL;
			}
			childs.clear();

			assert(threadIDToNode.erase(thread) == 1);

			if (!isExit() && this != threadRoot)
				pthread_kill(thread,SIGUSR2);

			pthread_join(thread,NULL);
			thread = 0;

			if(this->mutex != NULL){
				pthread_mutex_destroy(this->mutex);
				delete this->mutex;
				this->mutex = NULL;
			}
			if(this->cond != NULL){
				pthread_cond_destroy(this->cond);
				delete this->cond;
				this->cond = NULL;
			}
		}
		//cout << "~Node ==== end ==== " << this << endl;
	}

	inline pthread_t getThread()
	{
		return thread;
	}
	inline void setThread(pthread_t handle)
	{
		thread = handle;
	}
	inline Node *getFatherNode()
	{
		return father;
	}
	inline void setFatherNode(Node * node)
	{
		assert(this != node);
		father = node;
	}
	inline NODETYPE getNodeType()
	{
		return type;
	}
	inline void setNodeType(NODETYPE nType)
	{
		type = nType;
	}
	inline NODETYPE getNewNodeType()
	{
		return newType;
	}
	inline void setNewNodeType(NODETYPE nType)
	{
		newType = nType;
	}
	inline  void addChild(Node* node)
	{
		childs.push_back(node);
	}
	inline std::vector<Node*>& getChilds()
	{
		return childs;
	}

	inline void setNodeLength(NODELENGTH nType)
	{
		len = nType;
	}
	inline NODELENGTH getNodeLength()
	{
		return len;
	}

	inline bool getHasEmpty()
	{
		return hasEmpty;
	}

	inline void setHasEmpty(bool empty)
	{
		hasEmpty = empty;
	}

	//判断结点中的线程是否已经结束
	bool isExit()
	{
		if (this->thread == 0){
			return true;
		}
		int code = pthread_kill(this->thread,0);
		bool rtn = ( code == ESRCH || code == EINVAL);
		return rtn;
	}

	//执行当前结点,不同的结点有不同的实现方式
	virtual void excute() = 0;
	//判断结点是否正确的执行,有问题直接exit,全部结束返回true,否则返回false
	//在判断的同时将已经结束的子节点删除
	virtual bool isCorrectAndExit() = 0;

	pthread_cond_t* getCond()
	{
		return cond;
	}

	void setCond(pthread_cond_t* cond)
	{
		this->cond = cond;
	}

	pthread_mutex_t* getMutex()
	{
		return mutex;
	}

	void setMutex(pthread_mutex_t* mutex)
	{
		this->mutex = mutex;
	}

	bool isIsInit() const {
		return isInit;
	}

	void setIsInit(bool isInit) {
		this->isInit = isInit;
	}

protected:
//public:
	pthread_t thread;//结点所代表的线程
	Node* father;//父节点
	std::vector<Node*> childs;//孩子节点的集合
	NODETYPE type;//结点类型
	NODETYPE newType;//运行时被改变的结点类型
	NODELENGTH len;//结点的区间长度
	bool hasEmpty;//是否有到达empty的孩子

	pthread_mutex_t *mutex = nullptr;
	pthread_cond_t *cond = nullptr;

	bool isInit;
};

class AndNode :public Node
{
public:
	AndNode(pthread_t handle, Node * fatherNode, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, fatherNode, vec, mutex, cond, isInit)
	{
		newType = type = AND;
		//将and的孩子结点中的KEEP结点放在最后
		/*vector<Node*> keepVec;
		vector<Node*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			Node *node = *it;
			if (node->getNodeType() == KEEP)
			{
				keepVec.push_back(node);
				childs.erase(it);
				continue;
			}
			++it;
		}
		childs.insert(childs.end(), keepVec.begin(), keepVec.end());*/
	}
	AndNode(Node *node) : AndNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(), node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~AndNode(){}

	void excute()
	{
		if (isCorrectAndExit() || *pFalse)
		{
			newType = LEAF;
			return;
		}

		size_t i = 0;
		while (i<childs.size())
		{
			Node *child = childs[i];
			child->excute();

			if (child->getNodeType() != child->getNewNodeType())
			{
				childs[i] = getNewNode(child);
				continue;
			}
			++i;
		}
	}

	bool isCorrectAndExit()
	{
		vector<Node*>::iterator it = childs.begin();
		size_t emptyNum = 0, noneNum = 0;
		size_t size = childs.size();

		while (it != childs.end())
		{
			Node *node = *it;
			bool exited = node->isCorrectAndExit();

			if (node->getNodeLength() == EMPTY && node->getNodeType() == LEAF)
			{
				++emptyNum;
				hasEmpty = true;
			}
			else if (node->getNodeLength() == NONE)
				++noneNum;

			if (exited && node->getNodeLength() == EMPTY && node->getNodeType() == LEAF)
			{
				delete node; node = NULL;
				*it = NULL;
				childs.erase(it);
				continue;
			}
			++it;
		}

		if (!hasEmpty)
		{
			if (noneNum == size)
				len = NONE;
			else
				len = MORE;
			return false;
		}

		if (emptyNum + noneNum == size)
		{
			len = EMPTY;
			return true;
		}
		//cout << "FALSE!" << endl;
		//*pFalse = true;
		return false;
	}

	//and结点插入孩子时，将孩子插在第一个非keep结点的位置
	void insertChild(Node *child)
	{
		childs.insert(getInsertPos(), child);
	}

	void insertChild(vector<Node*> &child)
	{
		childs.insert(getInsertPos(), child.begin(),child.end());
	}

private:
	//根据childs找到插入位置,逆向查找最后一个非keep结点即可
	vector<Node*>::iterator getInsertPos()
	{
		vector<Node*>::reverse_iterator rit;
		for (rit = childs.rbegin(); rit != childs.rend(); ++rit)
			if ((*rit)->getNodeType() != KEEP)
				return ++(rit.base());

		return childs.begin();
	}
};

class PalNode :public Node
{
public:
	PalNode(pthread_t handle,  Node * node, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, node, vec, mutex, cond, isInit)
	{
		newType = type = PAL;
	}
	PalNode(Node *node) : PalNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(), node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~PalNode(){}

	void excute()
	{
		//cout << "Pal Node execute ==== start ==== " << endl;
		if (isCorrectAndExit() || *pFalse)
		{
			newType = LEAF;
			return;
		}

		vector<Node*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			Node *child = *it;
			child->excute();
			if (child->getNodeType() != child->getNewNodeType())
			{
				*it = getNewNode(child);
				continue;
			}
			++it;
		}
		//cout << "Pal Node execute ==== end ==== " << endl;
	}

	bool isCorrectAndExit()
	{
		vector<Node*>::iterator it = childs.begin();
		int emptyNum = 0;
		int size = childs.size();

		while ( it != childs.end() )
		{
			Node *node = *it;
			if (node->isCorrectAndExit() && node->getNodeType() == LEAF)
			{
				if (node->getNodeLength() == EMPTY)
				{
					++emptyNum;
					hasEmpty = true;
				}
				delete node;
				node = NULL;
				//cout << "delete " << node << endl;
				childs.erase(it);
				continue;
			}
			++it;
		}

		if (len == EMPTY)
			return true;

		if (emptyNum == size)
		{
			len = EMPTY;
			return true;
		}
		return false;
	}
};

class LeafNode :public Node
{
public:
	LeafNode(pthread_t handle, Node * node, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, node, vec, mutex, cond, isInit)
	{
		newType = type = LEAF;
	}
	LeafNode(Node *node) : LeafNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(), node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~LeafNode(){}

	void excute()
	{
		if (isExit() || *pFalse || len)
			return;

		while(!this->isInit){
			//usleep(1);
			sched_yield();
		}

		pthread_mutex_lock(this->getMutex());
		turn = this->thread;
		pthread_mutex_unlock(this->getMutex());
		pthread_cond_signal(this->getCond());

		while(turn != manager)
		{
			//usleep(1);
			sched_yield();
			if (isExit()){
				turn = manager;
				break;
			}
		}

	}

	bool isCorrectAndExit()
	{
		return isExit() || len;
	}
};

class PrjNode :public Node
{
	//投影中childs[0]代表公式中的q
public:
	PrjNode(pthread_t handle, Node * node, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, node, vec, mutex, cond, isInit)
	{
		newType = type = PRJ;
		extQ = true;
	}
	PrjNode(Node *node) : PrjNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(),node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~PrjNode(){}

	inline bool getHasExtQ()
	{
		return hasExtQ;
	}

	void excute()
	{
		if (isCorrectAndExit() || *pFalse)
		{
			newType = LEAF;
			return;
		}

		if (childs.size()<= 1)
		{
			extQ = true;
		}
		else
		{
			childs[1]->excute();
			while (childs[1]->getNodeType() != childs[1]->getNewNodeType())
			{
				childs[1] = getNewNode(childs[1]);
				childs[1]->excute();
			}

			while (childs[1]->getNodeLength() == EMPTY)
			{
				extQ = true;
				delete childs[1];
				childs.erase( ++childs.begin() );

				if (childs.size() == 1)
					break;

				childs[1]->excute();
				while (childs[1]->getNodeType() != childs[1]->getNewNodeType())
				{
					childs[1] = getNewNode(childs[1]);
					childs[1]->excute();
				}
			}
		}
		hasExtQ = extQ;
		if (extQ && !childs[0]->isCorrectAndExit())//
		{
			childs[0]->excute();
			while (childs[0]->getNodeType() != childs[0]->getNewNodeType())
			{
				childs[0] = getNewNode(childs[0]);
				childs[0]->excute();
			}
			extQ = false;
		}
	}

	bool isCorrectAndExit()
	{
		vector<Node*>::iterator it = childs.begin()+1;
		int exitNum = 0, size = childs.size();
		if (childs[0]->isExit())
			++exitNum;
		for (vector<Node*>::iterator it = ++childs.begin(); it != childs.end(); ++it)
		{
			if ((*it)->getNodeLength() == EMPTY && (*it)->getNodeType() == LEAF)
				++exitNum;
		}

		if (len == EMPTY)
			return true;

		if (exitNum == size)
		{
			len = EMPTY;
			return true;
		}
		return false;
	}

private:
	bool extQ;//是否执行Q
	bool hasExtQ;//当前状态是否已经加入Q
};

class KeepNode :public Node
{
public:
	KeepNode(pthread_t handle, Node * node, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, node, vec, mutex, cond, isInit)
	{
		type = newType = KEEP;
		len = NONE;
		father->setNodeLength(NONE);
		//if (father->getNodeType() == AND || father->getNewNodeType() == AND)
		//{//经过一些约定Keep结点在生成之前是不可能有孩子的
			//((AndNode*)father->getFatherNode())->insertChild(childs);
			//childs.clear();
			//return;
		//}
		//父节点不是AND,为它创造一个AND类型的父节点
		//AndNode * andNode = new AndNode(this);
		//father->getChilds().push_back(andNode);
		//andNode->getChilds().push_back(this);
		//father = andNode;
	}
	KeepNode(Node *node) : KeepNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(), node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~KeepNode(){}

	void excute()
	{
		if (isExit() || *pFalse){
			return;
		}

		while(!this->isInit){
			//usleep(1);
			sched_yield();
		}

		pthread_mutex_lock(this->getMutex());
		turn = this->thread;
		pthread_mutex_unlock(this->getMutex());
		pthread_cond_signal(this->getCond());

		while(turn != manager)
		{
			//usleep(1);
			sched_yield();
			if (isExit()){
				turn = manager;
				break;
			}
		}

		//执行过后将产生的线程结点加入到全局变量中,如果可以执行,会将这些结点加入到keep的父节点中
		for (vector<Node*>::iterator it = childs.begin(); it != childs.end(); ++it)
			(*it)->setFatherNode(this->father);

		keepChilds.insert(keepChilds.end(),childs.begin(),childs.end());
		childs.clear();
	}

	bool isCorrectAndExit()
	{
		return isExit();
	}
};

class AlwNode :public Node
{
public:
	AlwNode(pthread_t handle, Node * node, pthread_mutex_t *mutex, pthread_cond_t *cond, bool isInit, vector<Node*> vec = vector<Node*>()) : Node(handle, node, vec, mutex, cond, isInit)
	{
		type = newType = ALW;
		len = NONE;
		father->setNodeLength(NONE);
	}
	AlwNode(Node *node) : AlwNode(node->getThread(), node->getFatherNode(), node->getMutex(), node->getCond(), node->isIsInit(), node->getChilds()){
		node->setMutex(NULL);
		node->setCond(NULL);
	}
	~AlwNode(){}

	void excute()
	{
		if (isExit() || *pFalse){
			return;
		}

		while(!this->isInit){
			//usleep(1);
			sched_yield();
		}

		pthread_mutex_lock(this->getMutex());
		turn = this->thread;
		pthread_mutex_unlock(this->getMutex());
		pthread_cond_signal(this->getCond());

		while(turn != manager)
		{
			//usleep(1);
			sched_yield();
			if (isExit()){
				turn = manager;
				break;
			}
		}

		//将产生的线程结点加入到alw的父亲中
		for (vector<Node*>::iterator it = childs.begin(); it != childs.end(); ++it)
			(*it)->setFatherNode(this->father);

		father->getChilds().insert(father->getChilds().end(), childs.begin(), childs.end());

		if (childs.size() > 0)
		{
			Node * temp = father;
			while (temp != NULL && temp->getNodeLength() != MORE)
			{
				temp->setNodeLength(MORE);
				temp = temp->getFatherNode();
			}
		}
		childs.clear();
		//cout << "Alw Node execute ==== end ==== " << endl;
	}

	bool isCorrectAndExit()
	{
		return isExit();
	}
};

/*========================================================================================*/

//对当前状态排序的依据，算法类中sort函数的参数之一
//排序的结果是倒序的
bool Comp(const $$Element *a, const $$Element *b)
{
	if (a->priority > b->priority) return true;
	if (a->priority < b->priority) return false;
	if (a->priority != 4) return false;
	if (a->left.empty())
		return false;
	if (b->left.empty())
		return true;
	if (a->right.empty())
		return false;
	if (b->right.empty())
		return true;
	return (find(a->right.begin(), a->right.end(), b->left) != a->right.end());
}

//根据node的newType生成一个新类型的结点
//方法:创建新节点对象，将原结点对象的值复制，最后记得修改father结点中的成员:childs，以及删除旧结点
Node* getNewNode(Node *node)
{
	//cout << "getNewNode == " << node->getNewNodeType() << endl;
	Node *newNode = NULL;
	assert( threadIDToNode.erase(node->getThread()) == 1);

	switch (node->getNewNodeType())
	{
	case LEAF: //变为叶子节点之前要把自己的孩子清空
	{
		//for (vector<Node*>::iterator it = node->getChilds().begin(); it != node->getChilds().end(); it++)//
		//{
			//delete *it;
			//*it = NULL;
		//}

		node->getChilds().clear();
		newNode = new LeafNode(node);
		break;
	}
	case AND: newNode = new AndNode(node);break;
	case PAL: newNode = new PalNode(node);break;
	case PRJ: newNode = new PrjNode(node);break;
	case KEEP: newNode = new KeepNode(node);break;
	case ALW: newNode = new AlwNode(node);break;
	default:
		cout << "ERROR: In getNewNode , undefined fatherType:" << node->getNewNodeType() << endl;
		exit(0);
	}

	isCopied = true;
	//if (node != NULL)
		//delete node;
	node = newNode;
	isCopied = false;
	return node;
}

void updateNode(Node * node)
{
	if (node == NULL  || node->getNodeType() == node->getNewNodeType() || node == threadRoot){
		return;
	}

	vector<Node*> &childs = node->getFatherNode()->getChilds();

	size_t i;
	for (i = 0 ; childs[i] != node; ++i);
	Node * temp = childs[i];

	do{
		temp = getNewNode(temp);
		(temp->getFatherNode()->getChilds())[i] = temp;
		temp->excute();
	} while (temp->getNodeType() != temp->getNewNodeType());
}

void excuteKeepChilds()
{
	//cout << "excuteKeepChilds =====" << endl;
	vector<Node*>::size_type i = 0;
	while (i < keepChilds.size())
	{
		Node *node = keepChilds[i];
		Node *andFather = node;

		//找到keepchilds所在的and的根,不然无法判断结束，因为它自己的父亲状态可能是NONE,也就是取决于其它结点
		while ((andFather->getFatherNode() != NULL) && (andFather->getFatherNode()->getNodeType() == AND)){
			andFather = andFather->getFatherNode();
		}

		//cout << "excuteKeepChilds ==== 1 = " << andFather->getNodeLength() << endl;
		if (andFather->getNodeLength() == EMPTY )//如果父节点结束了keep的孩子就不执行
		{
			delete node;
			node = NULL;
			++i;
			continue;
		}
		//cout << "excuteKeepChilds ==== 1-1 ===  " << endl;
		node->getFatherNode()->getChilds().push_back(node);//将孩子加入到父节点中，参与以后的执行
		//cout << "excuteKeepChilds ==== 2 " << endl;
		Node *temp = node->getFatherNode();

		while (temp != NULL && temp->getNodeLength() != MORE)
		{
			temp->setNodeLength(MORE);
			temp = temp->getFatherNode();
		}

		//cout << "excuteKeepChilds ==== 1 ==== " << node->getFatherNode()->getNodeType() << endl;
		node->excute();
		updateNode(node);
		updateNode(node->getFatherNode());
		++i;
	}
	keepChilds.clear();
}

void sig_usr2(int signo)
{
	//cout << "sig_usr2" << endl;
	pthread_exit(NULL);
}

//主要分为3步:1.执行线程树,搜集排序使用的信息 2.排序 3.依次执行所有线程(肯定是叶子节点)
void* ManagerThread(void *para)
{

	while(turn != manager){
		//usleep(1);
		sched_yield();
	}

	if (threadRoot == NULL)
	{
		cout<< "threadRoot == NULL" << endl;
		return NULL;
	}

	vector<$$Element*>::size_type preSize;
	pthread_t id;
	bool hasExcuted = false;

	do{
		if (threadRoot->getNewNodeType() == LEAF || threadRoot->getNodeLength() == EMPTY)
		{
			break;
		}

		// 输出
		if (GlbOutput != NULL && IsOutput != NULL && stateNum != NULL)
		{
			if (!IsOutput()) exit(0);
			cout << "State[" << *stateNum << "]" ; GlbOutput(); cout << endl;
			if (stateNum != NULL) (*stateNum)++;
		}
		threadRoot->excute();
		changedNode = NULL;
		prjNode = NULL;

	SORT:
		if (prjNode != NULL)
		{
			prjNode->excute();
			prjNode = NULL;
		}

		pthread_mutex_lock(&pStackMutex);
		//sort($$prsnt.begin(), $$prsnt.end(), Comp);
		preSize = $$prsnt.size();
		pthread_mutex_unlock(&pStackMutex);

		while (!$$prsnt.empty())//执行经过排序之后的线程
		{
			hasExcuted = true;

			pthread_mutex_lock(&pStackMutex);
			$$Element* ele = *(--$$prsnt.end());
			$$prsnt.pop_back();
			preSize = $$prsnt.size();
			pthread_mutex_unlock(&pStackMutex);

			id = ele->threadId;

			map<pthread_t, Node*>::iterator it = threadIDToNode.find(id);
			if(it == threadIDToNode.end())
				assert(false);

			Node * tempNode = (*it).second;
			tempNode->excute();

			updateNode(tempNode);

			if (changedNode != NULL)
			{
				updateNode(changedNode);
				changedNode = NULL;
			}

			delete ele; ele = NULL;
			if ( preSize < $$prsnt.size() || prjNode != NULL)//执行之后加入了新的排序信息，需要重新排序执行
				goto SORT;

		}

		threadRoot->isCorrectAndExit();

		if (!keepChilds.empty())
		{
			excuteKeepChilds();
			goto SORT;
		}
		//cout << "next state" << endl;
		//状态数+1，输出全局变量信息
	} while (true);

	delete threadRoot;
	threadRoot = NULL;
	return NULL;
}


//当一段并行开始的时候，初始化调度线程，初始化线程树的根节点
void Init(int type)
{
	//signal(SIGUSR2,sig_usr2);

	if( 0 != pthread_create(&manager,NULL,&ManagerThread,NULL))
	{
		cerr << "ERROR: Create ManagerThread fail! at void Init(int type) !" << endl;
		exit(-1);
	}

	pthread_t thread = pthread_self();
	pthread_mutex_t *mutex = new pthread_mutex_t;
	pthread_mutex_init(mutex,NULL);
	pthread_cond_t *cond = new pthread_cond_t;
	pthread_cond_init(cond,NULL);

	switch (type)
	{
	case 0: threadRoot = new AndNode(thread, NULL, mutex, cond, false); break;
	case 1: threadRoot = new PalNode(thread, NULL, mutex, cond, false); break;
	case 2: threadRoot = new PrjNode(thread, NULL, mutex, cond, false); break;
	default:
		cerr << "ERROR: In Init() , undefined fatherType" << endl;
		exit(-1);
	}
}

//And Pal Prj Keep Alw Manager
//  0   1   2    3   4       5
//根据函数入口地址和参数创建线程，并为此线程生成Node*类型的结点，加入线程树中
void MyCreateThread(void* addr(void*),void* para,int type)
{
	if (type == 5)
		return;

	if (threadRoot == NULL)
		Init(type);

	//map<pthread_t,Node*>::iterator it = threadIDToNode.find(pthread_self());
	//if(it == threadIDToNode.end()) assert(false);

	Node *father = threadIDToNode.at(pthread_self());
	father->setNewNodeType(NODETYPE(type));

	if(addr == NULL)
		return;

	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr); //初始化 返回值为0 表示成功 -1 表示失败
	pthread_attr_setstacksize(&attr,PTHREAD_STACK_MIN); // 设置线程堆栈为最小值

	if(0 != pthread_create(&thread,&attr,addr,para))
	{
		cout << "Cannot create a thread , error " << endl;
		exit(0);
	}

	pthread_mutex_t *mutex = new pthread_mutex_t;
	pthread_mutex_init(mutex,NULL);
	pthread_cond_t *cond = new pthread_cond_t;
	pthread_cond_init(cond,NULL);

	Node *child = new LeafNode(thread, father, mutex, cond, false);
	father->getChilds().push_back(child);
}

//暂停一个线程，之后调度线程会阻塞掉它
void MyPauseThread()
{
	pthread_t tempID = pthread_self();
	Node *node = threadIDToNode.at(tempID);

	pthread_mutex_t *mutex = node->getMutex();
	pthread_cond_t *cond = node->getCond();

	pthread_mutex_lock(mutex);
	turn = manager;
	while(turn != tempID){
		pthread_cond_wait(cond,mutex);
	}
	pthread_mutex_unlock(mutex);
}

void MyPauseThreadInit()
{
	pthread_t tempID = pthread_self();

	Node *node = nullptr;
	while(true){
		try{
			node = threadIDToNode.at(tempID);
		}catch(...){
			node = nullptr;
		}
		if(node != nullptr){
			break;
		}
	}

	node->setIsInit(true);

	pthread_mutex_t *mutex = node->getMutex();
	pthread_cond_t *cond = node->getCond();

	pthread_mutex_lock(mutex);
	while(turn != tempID){
		pthread_cond_wait(cond,mutex);
	}
	pthread_mutex_unlock(mutex);
}

//这个函数只有主调线程最后会调用
void MyWaitForObject()
{
	static bool hasWaited = false;

	if (hasWaited){
		MyPauseThread();
		return;
	}

	hasWaited = true;

	turn = manager;

	// 等待Manger 线程结束
	pthread_join(manager,NULL);

	hasWaited = false;
	manager = 0;
	return;
}

void printPrsnt()//调试用，用来输出vector $$prsnt
{
	for (size_t i = 0; i < $$prsnt.size(); ++i)
	{
		$$Element * ele = $$prsnt[i];
		cout << "priority: " << ele->priority << "   "<< " left: " << ele->left << endl;
		cout << "right:";
		for (unsigned int j = 0; j < ele->right.size(); ++j)
			cout << " " << ele->right[j];
		cout << endl;
	}
}

void printIdNode()
{
	for (map<pthread_t, Node*>::iterator it = threadIDToNode.begin(); it != threadIDToNode.end(); ++it){
		cout << it->first << " - " << it->second << endl;
	}
}

//搜集信息
void $$Push(char * left,char * right,int priority)
{
	$$Element *ele = new $$Element();
	ele->left = string(left);
	string word(right);
	istringstream stream(word);
	while (stream >> word)
		ele->right.push_back(word);

	ele->priority = priority;
	ele->threadId = pthread_self();

	pthread_mutex_lock(&pStackMutex);
	$$prsnt.push_back(ele);
	pthread_mutex_unlock(&pStackMutex);
}

//获得MSV中的全局变量的信息
//参数1：状态数 参数2：全局变量输出函数
void GetMsvVar(int * sNum, void(*output)(), bool(*isout)())
{
	cout << "GetMsvVar" << endl;
	
	stateNum = sNum;
	GlbOutput = output;
	IsOutput = isout;
	pFalse = (bool*)malloc(sizeof(bool));
	*pFalse = false;
	//pFalse = pathFalse;
}

//根据子节点的NODELENGTH 修改当前节点
//返回值用于决定setNodeLength是否需要继续循环下去
void setAndLen(Node *node, NODELENGTH nLen)
{
	while (node != NULL && node->getNodeType() == AND)
	{
		vector<Node*> &childs = node->getChilds();
		unsigned int none = 0, emp = 0;
		for (vector<Node*>::iterator it = childs.begin(); it != childs.end(); ++it)
			if ((*it)->getNodeLength() == NONE)
				++none;
			else if ((*it)->getNodeLength() == EMPTY && (*it)->getNodeType() == LEAF)
				++emp;

		if (emp > 0)
			node->setHasEmpty(true);
		if (none == childs.size() && !node->getHasEmpty())
		{
			node->setNodeLength(NONE);
			node = node->getFatherNode();
			continue;
		}
		emp += node->getHasEmpty() ? none : 0;

		if (emp == childs.size())
		{
			node->setNodeLength(EMPTY);
			node->setNewNodeType(LEAF);
			//cout << "setAndLen ====   ==== changedNode=" << node << endl;
			changedNode = node;
			return;
		}
		return;
	}
}

//根据子节点的NODELENGTH 修改当前节点
//返回值用于决定setNodeLength是否需要继续循环下去
void setPalLen(Node *node, NODELENGTH nLen)//PAL结点的孩子结点区间长度不可能为NONE,这个在MSVCopmlier中有控制
{
	vector<Node*> &childs = node->getChilds();
	unsigned int none = 0,emp = 0;
	for (vector<Node*>::iterator it = childs.begin(); it != childs.end(); ++it)
		if ((*it)->getNodeLength() == NONE)
			++none;
		else if ((*it)->getNodeLength() == EMPTY && (*it)->getNodeType() == LEAF)
			++emp;
	if (none == childs.size() )
	{
		node->setNodeLength(NONE);
		return ;
	}

	if (emp + none == childs.size())
	{
		node->setNodeLength(EMPTY);
		node->setNewNodeType(LEAF);
		changedNode = node;
	}
	return ;
}

//根据子节点的NODELENGTH 修改当前节点
//将更改过的结点加入changedNode中
void setPrjLen(Node *father, Node *child, NODELENGTH nLen)//PRJ结点的孩子结点区间长度不可能为NONE,这个在MSVCopmlier中有控制
{
	vector<Node*> &childs = father->getChilds();
	vector<Node*>::iterator pos;
	unsigned int sum = 0;
	for (vector<Node*>::iterator it = childs.begin(); it != childs.end(); ++it)
	{
		if (*it == child)
			pos = it + 1;

		if ((*it)->getNodeLength() != MORE && (*it)->getNodeType() == LEAF)
			++sum;
	}
	if (sum == childs.size())//投影结点孩子的区间长度全部为empty
	{
		father->setNodeLength(EMPTY);
		father->setNewNodeType(LEAF);
		return ;
	}
	if (childs[0] == child )//说明Q结点置为empty,
		return ;

	if (pos == childs.end())
		return ;
	if (!((PrjNode*)father)->getHasExtQ())
	{
		prjNode = father;//将下一个p的第一状态取出
	}
}

//设置线程树中结点的区间长度，由于more是默认的，因此参数值为enmpty或none
//根据区间长度修改父节点的长度，投影为empty时要执行下一个p的第一个状态
void setNodeLength(int type)
{

	map<pthread_t, Node*>::iterator it = threadIDToNode.find(pthread_self());
	if(it == threadIDToNode.end())
		assert(false);

	Node * node = (*it).second;
	//if (node->getNodeLength() != MORE)
		//return;
	node->setNodeLength(NODELENGTH(type));
	Node * temp = node->getFatherNode();
	if (temp != NULL )
	{
		switch (temp->getNodeType())
		{
		case AND:  setAndLen(temp, NODELENGTH(type)); break;
		case PAL:  setPalLen(temp, NODELENGTH(type)); break;
		case PRJ: setPrjLen(temp, node, NODELENGTH(type)); break;
		default:  break;
		}
	}
}
