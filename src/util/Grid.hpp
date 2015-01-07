#pragma once

#include "../PopSS.h"

template<typename T>
class Grid {
public:
	Grid() {
		this->width = 0;
		this->height = 0;
		this->data = NULL;
	}

	Grid(int size) : Grid(size, size) { }

	Grid(int width, int height) {
		assert(width > 0 && height > 0);

		this->width = width;
		this->height = height;
		this->data = new T[width * height];
	}

	Grid & operator=(const Grid &rhs) {
		if (this != &rhs) {
			this->width = rhs.width;
			this->height = rhs.height;
			this->data = new T[this->width * this->height];
			for (int i = 0; i < this->width * this->height; i++)
				this->data[i] = rhs.data[i];
		}

		return *this;
	}

	~Grid() {
		if (this->data != NULL)
			delete[] this->data;
	}

	T Get(int x, int y) const {
		return this->data[this->GetIndex(x, y)];
	}

	void Set(int x, int y, T value) {
		this->data[this->GetIndex(x, y)] = value;
	}

	void Clear(T value) {
		for (int i = 0; i < this->width * this->height; i++)
			this->data[i] = value;
	}

	int GetWidth() const { return this->width; }
	int GetHeight() const { return this->height; }
	T *GetData() { return this->data; }
	size_t GetDataSize() const { return this->width * this->height * sizeof(T); }

private:
	int width, height;
	T *data;

	int GetIndex(int x, int y) const {
		assert(x >= 0 && x < this->width);
		assert(y >= 0 && y < this->height);

		return x + y * this->width;
	}
};