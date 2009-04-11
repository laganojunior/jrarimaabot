#ifndef __JR_MAXHEAP_H__
#define __JR_MAXHEAP_H__

#include <vector>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Takes an array and converts it in place to a max-heap data structure, which
//is basically a binary tree, where each parent is greater than each of its
//children. This guarantees that the greatest element is at the top (index 0)
//////////////////////////////////////////////////////////////////////////////
template<class T>
void maxHeapCreate(vector<T>& array)
{
    for (int i = array.size() - 1; i >= 0; --i)
    {
        maxHeapSink(array, i);
    }
}

//////////////////////////////////////////////////////////////////////////////
//Assuming that the given index refers to a node in a tree s.t. its children
//are heaps, this function ensures that the node referred to is sunk
//(put lower into the tree) until the node's placement keeps the heap's
//structure.
//////////////////////////////////////////////////////////////////////////////
template<class T>
void maxHeapSink(vector<T>& heap, unsigned int index)
{
    if (index >= heap.size() - 1)
        return;

    int left  = (index + 1) * 2 - 1;
    int right = (index + 1) * 2; 

    //Node is out of place and the left child is greater
    if (left < heap.size() &&  
        heap[left] > heap[index] && heap[left] > heap[right])
    {
        //swap the node and the left child, and then sink the node again
        T temp;
        temp = heap[left];
        heap[left] = heap[index];
        heap[index] = temp;

        maxHeapSink(heap, left);
    }  

    //Node is out of place and the right child is greater
    if (right < heap.size() &&
        heap[right] > heap[index] && heap[right] > heap[left])
    {
        //swap the node and the right child, and then sink the node again
        T temp;
        temp = heap[right];
        heap[right] = heap[index];
        heap[index] = temp;

        maxHeapSink(heap, right);
    }  
}

//////////////////////////////////////////////////////////////////////////////
//Assuming that the given index refers to a node that is in a tree that would
//be a heap if that node is removed, this floats that node (moves it up the
//tree) until it is placed in a position that preserves heap structure.
//////////////////////////////////////////////////////////////////////////////
template<class T>
void maxHeapFloat(vector<T>& heap, unsigned int index)
{
    if (index <= 0)
        return;

    int parent = (index + 1) / 2 - 1;
    
    //Node is out of place
    if (heap[index] > heap[parent])
    {
        //swap the node and the paren, and then float the node again
        T temp;
        temp = heap[index];
        heap[index] = heap[parent];
        heap[parent] = temp;

        maxHeapFloat(heap, parent);
    }  
}
        
//////////////////////////////////////////////////////////////////////////////
//Returns the top of the heap, which is the maximum element of the heap and
//removes, while preserving heap structure.
//////////////////////////////////////////////////////////////////////////////
template <class T>
T maxHeapGetTopAndRemove(vector<T>& heap)
{
    T top = heap[0];

    //Now set the heap's size lower and preserve heap structure
    heap[0] = heap[heap.size() - 1];
    maxHeapSink(heap, 0);

    heap.resize(heap.size() - 1);   

    return top; 
}

#endif
