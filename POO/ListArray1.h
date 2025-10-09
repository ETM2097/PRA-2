#include "List.h"
#include <ostream>

template <typename T>
class ListArray : public List<T> {
        private:
                T* arr;
                int max;
                int n;
                static const int MINSIZE = 2;
        public: 
                ListArray() : n(0), max(MINSIZE) {
                        arr = new T[MINSIZE];
                }
                ~ListArray() {
                        delete[] arr;
                }
                int size() {
                        return n;
                }
                void insert(int pos, T e){
                    if (pos < 0 || pos > size()) {
                        throw std::out_of_range("Index out of range");
                    }
                    if (n == max) {
                        resize(max * 2);
                    }
                    for (int i = n; i > pos; i--) {
                        arr[i] = arr[i - 1];
                    }
                    arr[pos] = e;
                    n++;
                }

                void resize(int newSize) {
                    if(newSize > MINSIZE){
                        if (max > newSize){
                            T* newArr = new T[newSize];
                            for (int i = 0; i < n; i++) {
                                newArr[i] = arr[i];
                            }
                            delete[] arr;
                            arr = newArr;
                            max = newSize;
                        }
                        if (max < newSize){
                            T* newArr = new T[newSize];
                            for (int i = 0; i < n; i++) {
                                newArr[i] = arr[i];
                            }
                            delete[] arr;
                            arr = newArr;
                            max = newSize;
                        }
                    }
                    else throw std::length_error("New size must be greater than MINSIZE");
                }
}; 
