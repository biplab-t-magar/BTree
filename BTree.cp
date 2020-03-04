#include <iostream>
#include <cmath>
using namespace std;

template<class T>
class BTreeNode;

//struct used to pass info up the tree when splitting nodes during insertion
template <class T>
struct SplitNodeData {
	BTreeNode<T> *newNode; //The new node created after the split (the right partition, in this case). The pointer to the other node created after the split is the same as that of the pre-split node
	T key; //The key passed up the tree after the splitting of the nodes
};


template<class T>
class BTreeNode {
private:
	T *keys; //array of keys
	BTreeNode<T> **children; //array of children nodes
	int numOfKeys; //current number of keys
	int maxDegree; //Maximum degree (maximum number of child nodes per node)
	bool leaf; //Is the node a leaf node
	int searchKeyPosition(T k); //returns the position of the first key that is greater than or equal to the given key
	void insertToNonFullNode(T k, BTreeNode<T> *newNode = NULL);
	SplitNodeData<T>* insertToFullNodeAndSplit(T k, BTreeNode<T> *newNode = NULL); //returns data after splitting (including pointer to new right child node)
	void deleteKey(T key);
	void removeFromLeafNode(int position);
	void removeFromNonLeafNode(int position);
	int getPred(int position);
	int getSucc(int position);
	void fill(int position);
	void borrowFromPrev(int position);
	void borrowFromNext(int position);
	void merge(int position);
public:
	BTreeNode(int maxDegree, bool leaf);
	template <class T>
	friend class BTree;
	~BTreeNode();
};


template <class T>
void BTreeNode<T>::deleteKey(T key) {
	int position = searchKeyPosition(key);

	// If the key to be removed is present in this node
	if (position < numOfKeys && keys[position] == key) {
		if (leaf)
			removeFromLeafNode(position);
		else
			removeFromNonLeafNode(position);
	}
	else {

		// If this node is a leaf node, then the key is not present in tree
		if (leaf)
		{
			return;
		}

		// The flag indicates whether the key is present in the sub-tree rooted with the last child of this node
		bool flag = (position == numOfKeys);

		// If the child where the key is supposed to exist has less than minimum keys,
		// we fill that child
		if (children[position]->numOfKeys < ceil(((double)maxDegree - 1) / 2)) {
			fill(position);
		}

		if (flag && position > numOfKeys)
			children[position - 1]->deleteKey(key);
		else
			children[position]->deleteKey(key);
	}
	return;
}

template <class T>
void BTreeNode<T>::removeFromLeafNode(int position) {

	for (int i = position + 1; i < numOfKeys; i++)
		keys[i - 1] = keys[i];

	numOfKeys--;

}

template <class T>
void BTreeNode<T>::removeFromNonLeafNode(int position) {

	int k = keys[position];

	if (children[position]->numOfKeys >= ceil(((double)maxDegree - 1) / 2))	{
		int pred = getPred(position);
		keys[position] = pred;
		children[position]->deleteKey(pred);
	}

	else if (children[position + 1]->numOfKeys >= ceil(((double)maxDegree - 1) / 2))	{
		int succ = getSucc(position);
		keys[position] = succ;
		children[position + 1]->deleteKey(succ);
	}

	else
	{
		merge(position);
		children[position]->deleteKey(k);
	}
	return;
}

template <class T>
int BTreeNode<T>::getPred(int position) {
	BTreeNode<T> *cur = children[position];
	while (!cur->leaf)
		cur = cur->children[cur->numOfKeys];

	// Return the last key of the leaf
	return cur->keys[cur->numOfKeys - 1];
}

template <class T>
int BTreeNode<T>::getSucc(int position){

	BTreeNode<T> *cur = children[position + 1];
	while (!cur->leaf)
		cur = cur->children[0];

	// Return the first key of the leaf
	return cur->keys[0];
}

template <class T>
void BTreeNode<T>::fill(int position){


	if (position != 0 && children[position - 1]->numOfKeys >= ceil(((double)maxDegree - 1) / 2))
		borrowFromPrev(position);


	else if (position != numOfKeys && children[position + 1]->numOfKeys >= ceil(((double)maxDegree - 1) / 2))
		borrowFromNext(position);

	else{
		if (position != numOfKeys)
			merge(position);
		else
			merge(position - 1);
	}
}

template <class T>
void BTreeNode<T>::borrowFromPrev(int position){

	BTreeNode *child = children[position];
	BTreeNode *sibling = children[position - 1];


	for (int i = child->numOfKeys - 1; i >= 0; --i)
		child->keys[i + 1] = child->keys[i];

	if (!child->leaf)
	{
		for (int i = child->numOfKeys; i >= 0; --i)
			child->children[i + 1] = child->children[i];
	}

	child->keys[0] = keys[position - 1];

	if (!child->leaf)
		child->children[0] = sibling->children[sibling->numOfKeys];


	keys[position - 1] = sibling->keys[sibling->numOfKeys - 1];

	child->numOfKeys += 1;
	sibling->numOfKeys -= 1;

	return;
}

template <class T>
void BTreeNode<T>::borrowFromNext(int position){

	BTreeNode *child = children[position];
	BTreeNode *sibling = children[position + 1];

	child->keys[(child->numOfKeys)] = keys[position];


	if (!(child->leaf))
		child->children[(child->numOfKeys) + 1] = sibling->children[0];

	keys[position] = sibling->keys[0];

	for (int i = 1; i < sibling->numOfKeys; ++i)
		sibling->keys[i - 1] = sibling->keys[i];

	if (!sibling->leaf)
	{
		for (int i = 1; i <= sibling->numOfKeys; ++i)
			sibling->children[i - 1] = sibling->children[i];
	}


	child->numOfKeys += 1;
	sibling->numOfKeys -= 1;

	return;
}

template <class T>
void BTreeNode<T>::merge(int position){
	BTreeNode *child = children[position];
	BTreeNode *sibling = children[position + 1];


	child->keys[(int)ceil(((double)maxDegree - 1) / 2) - 1] = keys[position];

	for (int i = 0; i < sibling->numOfKeys; ++i)
		child->keys[(int)(i + ceil(((double)maxDegree - 1) / 2))] = sibling->keys[i];

	if (!child->leaf)
	{
		for (int i = 0; i <= sibling->numOfKeys; ++i)
			child->children[(int)(i + ceil(((double)maxDegree - 1) / 2))] = sibling->children[i];
	}


	for (int i = position + 1; i < numOfKeys; ++i)
		keys[i - 1] = keys[i];


	for (int i = position + 2; i <= numOfKeys; ++i)
		children[i - 1] = children[i];

	child->numOfKeys += sibling->numOfKeys + 1;
	numOfKeys--;

	delete(sibling);
	return;
}

template <class T>
BTreeNode<T>::BTreeNode(int maxDegree, bool leaf) {
	if (maxDegree < 2) {
		throw "Invalid degree for BTree";
	}
	this->leaf = leaf;
	this->maxDegree = maxDegree;
	this->numOfKeys = 0;
	this->keys = new T[maxDegree - 1];
	this->children = new BTreeNode<T> *[maxDegree];
	for (int i = 0; i < maxDegree; i++) {
		children[i] = NULL;
	}
}


template <class T>
BTreeNode<T>::~BTreeNode() {
	delete[] keys;
	delete[] children;
}

template <class T>
int BTreeNode<T>::searchKeyPosition(T k) {
	if (numOfKeys == 0) {
		return 0;
	}
	int begin = 0;
	int end = numOfKeys - 1;
	int mid;
	while (begin <= end) {
		mid = (begin + end) / 2;
		if (keys[mid] == k) {
			return mid;
		}
		else if (keys[mid] > k) {
			end = mid - 1;
		}
		else {
			begin = mid + 1;
		}
	}
	return begin; //note: if k is greater than all the keys in the node, the return value is the index of the last key + 1
}

template <class T>
void BTreeNode<T>::insertToNonFullNode(T k, BTreeNode<T> *newNode) {
	if (numOfKeys >= maxDegree - 1) {
		throw "Error: cannot insert key to a full node without splitting";
	}

	//add the key first
	//find the position of the new key
	int position = searchKeyPosition(k);
	//move every key past that position to the right
	for (int i = numOfKeys - 1; i >= position; i--) {
		this->keys[i + 1] = this->keys[i];
	}
	//add key to correct position
	this->keys[position] = k;

	if (this->leaf != true) {
		//if not leaf node
		//add the new child node (it has to be the right child out of the two split nodes)
		for (int i = numOfKeys; i >= position + 1; i--) {
			this->children[i + 1] = this->children[i];
		}
		this->children[position + 1] = newNode;
	}

	this->numOfKeys++;

}

template <class T>
SplitNodeData<T>* BTreeNode<T>::insertToFullNodeAndSplit(T k, BTreeNode<T> *newNode) {
	if (numOfKeys != maxDegree - 1) {
		throw "Error: cannot split if node is not full";
	}

	struct SplitNodeData<T>* splitNodeInfo = new SplitNodeData<T>;

	//first, search for the position to which the new key would fit in the current node
	int position = searchKeyPosition(k);

	//create a temp array that allows space for one extra key
	T *tempKeys = new T[maxDegree];

	//now, populate the new array with the keys in sorted order
	for (int i = 0; i < position; i++) {
		tempKeys[i] = keys[i];
	}
	tempKeys[position] = k;
	for (int i = position + 1; i < maxDegree; i++) {
		tempKeys[i] = keys[i - 1];
	}

	//now, split tempArray evenly to create the new array and add the middle node into SplitNodeData struct to pass into the node above
	//create left child node and make it replace the current node
	for (int i = 0; i < maxDegree / 2; i++) {
		this->keys[i] = tempKeys[i];
	}
	//empty the rest of the values in the node
	for (int i = maxDegree / 2; i < maxDegree - 1; i++) {
		this->keys[i] = NULL;
	}
	this->numOfKeys = maxDegree / 2;
	//add the middle key to SplitNodeData
	splitNodeInfo->key = tempKeys[maxDegree / 2];
	//create the new right child node
	BTreeNode<T>* rightChild = new BTreeNode<T>(maxDegree, true);
	for (int i = maxDegree / 2 + 1, j = 0; i < maxDegree; i++, j++) {
		rightChild->keys[j] = tempKeys[i];
	}
	rightChild->numOfKeys = (maxDegree - 1) / 2;

	//if this node is not a leaf node
	if (!this->leaf) {
		if (newNode == NULL) {
			throw "Error: A key cannot be directly inserted to a non-leaf node. The new key has to be passed up from a child node. The argument for the newly split child's right half is missing";
		}

		//create a temp array that allows space for one extra child
		BTreeNode<T> **tempChildren = new BTreeNode<T> *[maxDegree + 1];

		//now, populate the new array with the children in correct order
		for (int i = 0; i < position + 1; i++) {
			tempChildren[i] = this->children[i];
		}
		tempChildren[position + 1] = newNode;
		for (int i = position + 2; i < maxDegree + 1; i++) {
			tempChildren[i] = this->children[i - 1];
		}
		//now, split this tempnode like earlier
		//add first half of all children to current node to complete creation of left node
		for (int i = 0; i <= maxDegree / 2; i++) {
			this->children[i] = tempChildren[i];
		}
		//empty the rest of the values in this left child node
		for (int i = maxDegree / 2 + 1; i < maxDegree; i++) {
			this->children[i] = NULL;
		}
		//add the children for the right child node
		for (int i = maxDegree / 2 + 1, j = 0; i < maxDegree + 1; i++, j++) {
			rightChild->children[j] = tempChildren[i];
		}

		rightChild->leaf = false;
		delete tempChildren;
	}
	delete tempKeys;
	//Add right child node to SplitNodeData
	splitNodeInfo->newNode = rightChild;
	return splitNodeInfo;
}


template <class T>
class BTree {
private:
	BTreeNode<T> *rootNode;
	int maxDegree;
	SplitNodeData<T>* insertImplementation(T k, BTreeNode<T>* node); //utility function to insert node into a subtree
	void createNewRootNode(T key, BTreeNode<T>* rightChildNode);
public:
	BTree(int maxDegree);
	void traverse(BTreeNode<T>* node = NULL);
	BTreeNode<T>* search(T key, BTreeNode<T>* node = NULL);
	void insert(T k);
	void deleteKey(T key, BTreeNode<T>* node = NULL);
};

template <class T>
BTree<T>::BTree(int maxDegree) {
	if (maxDegree < 2) {
		throw "Invalid max degree for BTree";
	}
	rootNode = NULL;
	this->maxDegree = maxDegree;
}

template <class T>
void BTree<T>::deleteKey(T key, BTreeNode<T>* node) {
	if (rootNode == NULL) {
		return;
	}

	rootNode->deleteKey(key);

	//if the root node has 0 keys, make its first child the new root if it has a child
	//if it has no child, set it to NULL
	if (rootNode->numOfKeys == 0) {
		BTreeNode<T>* tempNode = rootNode;
		if (rootNode->leaf) {
			rootNode = NULL;
		}
		else {
			rootNode = rootNode->children[0];
		}
		delete tempNode;
	}
}

template <class T>
void BTree<T>::traverse(BTreeNode<T>* node) {
	if (node == NULL) {
		node = rootNode;
	}
	int i;

	//inorder traversal
	for (i = 0; i < node->numOfKeys; i++) {
		if (!node->leaf) {
			traverse(node->children[i]);
		}
		cout << " " << node->keys[i];
	}
	if (!node->leaf) {
		traverse(node->children[node->numOfKeys]);
	}

}

template <class T>
BTreeNode<T>* BTree<T>::search(T k, BTreeNode<T>* node) {
	if (node == NULL) {
		node = rootNode;
	}
	//find the key that is greater than or equal to k using binary search
	int position = node->searchKeyPosition(k);

	//return if key found
	if (position < node->numOfKeys && k == node->keys[position]) {
		return node;
	}

	//return NULL if current node has no children, and thus all searches have been exhausted
	if (node->leaf) {
		return NULL;
	}

	return search(k, node->children[position]);
}

template <class T>
void BTree<T>::insert(T k) {
	//if the subtree is empty
	if (rootNode == NULL) {
		rootNode = new BTreeNode<T>(maxDegree, true);
		rootNode->keys[0] = k;
		rootNode->numOfKeys = 1;
		return;
	}

	insertImplementation(k, rootNode);
}

template <class T>
void BTree<T>::createNewRootNode(T key, BTreeNode<T>* rightChildNode) {
	BTreeNode<T> *newRoot = new BTreeNode<T>(maxDegree, false);
	newRoot->numOfKeys = 1;
	newRoot->keys[0] = key;
	//left child is current root and right child is in argument
	newRoot->children[0] = this->rootNode;
	newRoot->children[1] = rightChildNode;
	this->rootNode = newRoot;
}

template <class T>
SplitNodeData<T>* BTree<T>::insertImplementation(T k, BTreeNode<T>* node) {
	struct SplitNodeData<T>* childNodeData = NULL;
	//if node is a leaf
	if (node->leaf) {
		//if you reach a leaf and the node is not full
		if (node->numOfKeys < maxDegree - 1) {
			node->insertToNonFullNode(k);
			return NULL;
		}
		else {
			//if the node is full, split node

			childNodeData = node->insertToFullNodeAndSplit(k);

			//if the node is the root node (the initial root node is the only root node that is also a leaf), create a new root node
			if (node == rootNode) {
				createNewRootNode(childNodeData->key, childNodeData->newNode);
				delete childNodeData;
			}
			//else, send information to parent node
			return childNodeData;

		}
	}
	else { // if not a leaf, go down to the child node
		childNodeData = insertImplementation(k, node->children[node->searchKeyPosition(k)]);
		//if key was successfully added without splitting, then return
		if (childNodeData == NULL) {
			return NULL;
		}
		else {
			//if the current node is not full
			if (node->numOfKeys < maxDegree - 1) {
				//add the new key and the new child
				node->insertToNonFullNode(childNodeData->key, childNodeData->newNode);
				delete childNodeData;
				return NULL;
			}
			else { //if the current node is full too
				struct SplitNodeData<T>* currentNodeData = node->insertToFullNodeAndSplit(childNodeData->key, childNodeData->newNode);
				delete childNodeData;
				//if the current node is the root node, create a new root node, with the children being the split version of the current root node
				if (node == this->rootNode) {
					createNewRootNode(currentNodeData->key, currentNodeData->newNode);
					delete currentNodeData;
					return NULL;
				}
				else {
					return currentNodeData;
				}

			}
		}
	}
}




int main() {
	BTree<int> b(3);
	b.insert(4);
	b.insert(9);
	b.insert(3);
	b.insert(145);
	b.insert(67);
	b.insert(20);
	b.insert(67);
	b.insert(67);
	b.insert(55);
	b.insert(67);
	b.insert(0);
	b.traverse();

	if (b.search(4) != NULL) {
		cout << endl << "Found 4" << endl;
	}

	if (b.search(99) == NULL) {
		cout << "Could not find 99" << endl;
	}

	if (b.search(67) != NULL) {
		cout << "Found 67" << endl;
	}

	b.deleteKey(4);
	if (b.search(4) == NULL) {
		cout << "Could not find 4" << endl;
	}

	b.deleteKey(20);
	if (b.search(20) == NULL) {
		cout << "Could not find 20" << endl;
	}

	b.insert(4);
	if (b.search(4) != NULL) {
		cout << "found 4" << endl;
	}

	b.deleteKey(67);
	if (b.search(67) == NULL) {
		cout << "could not find 67" << endl;
	}

	b.traverse();
	b.deleteKey(4);
	cout << endl;
	b.traverse();
	return 0;
}
