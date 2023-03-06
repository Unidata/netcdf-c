// The MIT License (MIT)
// Simplistic Binary Streams 1.0.3
// Copyright (C) 2014 - 2019, by Wong Shao Voon (shaovoon@yahoo.com)
//
// http://opensource.org/licenses/MIT
//
// version 0.9.2   : Optimize mem_istream constructor for const char*
// version 0.9.3   : Optimize mem_ostream vector insert
// version 0.9.4   : New ptr_istream class
// version 0.9.5   : Add Endianness Swap with compile time check
// version 0.9.6   : Using C File APIs, instead of STL file streams
// version 0.9.7   : Add memfile_istream
// version 0.9.8   : Fix GCC and Clang template errors
// version 0.9.9   : Fix bug of getting previous value when reading empty string
// version 1.0.0   : Fix buffer overrun bug when reading string (reported by imtrobin)
// version 1.0.1   : Fix memfile_istream tellg and seekg bug reported by macxfadz, 
//                   use is_arithmetic instead of is_integral to determine swapping
// version 1.0.2   : Add overloaded open functions that take in file parameter in 
//                   wide char type.(only available on win32)
// version 1.0.3   : Remove <iostream> header
// version 1.0.4   : Fixed file_istream's seekg() and added writeat() to mem_ostream and memfile_ostream. Thanks Festering from CodeProject.

#ifndef SimpleBinStream_H
#define SimpleBinStream_H

#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <stdint.h>
#include <cstdio>

namespace nczarr
{
	enum class Endian
	{
		Big,
		Little
	};
	using BigEndian = std::integral_constant<Endian, Endian::Big>;
	using LittleEndian = std::integral_constant<Endian, Endian::Little>;

	struct SizeOf1 { };
	struct SizeOf2 { };
	struct SizeOf4 { };
	struct SizeOf8 { };
	struct UnknownSize { };

	template<typename T>
	void swap_endian(T& ui, UnknownSize)
	{
	}

	template<typename T>
	void swap_endian(T& ui, SizeOf1)
	{
	}

	template<typename T>
	void swap_endian(T& ui, SizeOf8)
	{
		union EightBytes
		{
			T ui;
			uint8_t arr[8];
		};

		EightBytes fb;
		fb.ui = ui;
		// swap the endian
		std::swap(fb.arr[0], fb.arr[7]);
		std::swap(fb.arr[1], fb.arr[6]);
		std::swap(fb.arr[2], fb.arr[5]);
		std::swap(fb.arr[3], fb.arr[4]);

		ui = fb.ui;
	}

	template<typename T>
	void swap_endian(T& ui, SizeOf4)
	{
		union FourBytes
		{
			T ui;
			uint8_t arr[4];
		};

		FourBytes fb;
		fb.ui = ui;
		// swap the endian
		std::swap(fb.arr[0], fb.arr[3]);
		std::swap(fb.arr[1], fb.arr[2]);

		ui = fb.ui;
	}

	template<typename T>
	void swap_endian(T& ui, SizeOf2)
	{
		union TwoBytes
		{
			T ui;
			uint8_t arr[2];
		};

		TwoBytes fb;
		fb.ui = ui;
		// swap the endian
		std::swap(fb.arr[0], fb.arr[1]);

		ui = fb.ui;
	}

	template <class T>
	using number_type =
		typename std::conditional<
		sizeof(T) == 1,
		SizeOf1,
		typename std::conditional<
		sizeof(T) == 2,
		SizeOf2,
		typename std::conditional<
		sizeof(T) == 4,
		SizeOf4,
		typename std::conditional<
		sizeof(T) == 8,
		SizeOf8,
		UnknownSize
		>::type
		>::type
		>::type
		>::type;
	
	template<typename T>
	void swap_if_arithmetic(T& val, std::true_type)
	{
		swap_endian(val, number_type<T>());
	}
	
	template<typename T>
	void swap_if_arithmetic(T& val, std::false_type)
	{
		// T is not arithmetic so do nothing
	}

	template<typename T>
	void swap_endian_if_same_endian_is_false(T& val, std::false_type)
	{
		std::is_arithmetic<T> is_integral_type;

		swap_if_arithmetic(val, is_integral_type);
	}
	
	template<typename T>
	void swap_endian_if_same_endian_is_false(T& val, std::true_type)
	{
		// same endian so do nothing.
	}

template<typename same_endian_type>
class file_istream
{
public:
	file_istream() : input_file_ptr(nullptr), file_size(0L), read_length(0L) {}
	file_istream(const char * file) : input_file_ptr(nullptr), file_size(0L), read_length(0L)
	{
		open(file);
	}
#ifdef _MSC_VER
	file_istream(const wchar_t * file) : input_file_ptr(nullptr), file_size(0L), read_length(0L)
	{
		open(file);
	}
#endif
	~file_istream()
	{
		close();
	}
	void open(const char * file)
	{
		close();
#ifdef _MSC_VER
		input_file_ptr = nullptr;
		fopen_s(&input_file_ptr, file, "rb");
#else
		input_file_ptr = std::fopen(file, "rb");
#endif
		compute_length();
	}
#ifdef _MSC_VER
	void open(const wchar_t * file)
	{
		close();
		input_file_ptr = nullptr;
		_wfopen_s(&input_file_ptr, file, L"rb");
		compute_length();
	}
#endif
	void close()
	{
		if (input_file_ptr)
		{
			fclose(input_file_ptr);
			input_file_ptr = nullptr;
		}
	}
	bool is_open()
	{
		return (input_file_ptr != nullptr);
	}
	long file_length() const
	{
		return file_size;
	}
	// http://www.cplusplus.com/reference/cstdio/feof/
	// stream's internal position indicator may point to the end-of-file for the 
	// next operation, but still, the end-of-file indicator may not be set until 
	// an operation attempts to read at that point.
	bool eof() const // not using feof(), see above
	{
		return read_length >= file_size;
	}
	long tellg() const
	{
		return std::ftell(input_file_ptr);
	}
	void seekg (long pos)
	{
		std::fseek(input_file_ptr, pos, SEEK_SET);
		read_length = pos;
	}
	void seekg (long offset, int way)
	{
		std::fseek(input_file_ptr, offset, way);
		if (way == SEEK_END)
			read_length = file_size - offset;
		else if (way == SEEK_CUR)
			read_length += offset;
		else
			read_length = offset;
	}

	template<typename T>
	void read(T& t)
	{
		if(std::fread(reinterpret_cast<void*>(&t), sizeof(T), 1, input_file_ptr) != 1)
		{
			throw std::runtime_error("Read Error!");
		}
		read_length += sizeof(T);
		nczarr::swap_endian_if_same_endian_is_false(t, m_same_type);
	}
	void read(typename std::vector<char>& vec)
	{
		if (std::fread(reinterpret_cast<void*>(&vec[0]), vec.size(), 1, input_file_ptr) != 1)
		{
			throw std::runtime_error("Read Error!");
		}
		read_length += vec.size();
	}
	void read(char* p, size_t size)
	{
		if (std::fread(reinterpret_cast<void*>(p), size, 1, input_file_ptr) != 1)
		{
			throw std::runtime_error("Read Error!");
		}
		read_length += size;
	}
private:
	void compute_length()
	{
		seekg(0, SEEK_END);
		file_size = tellg();
		seekg(0, SEEK_SET);
	}

	std::FILE* input_file_ptr;
	long file_size;
	long read_length;
	same_endian_type m_same_type;
};

template<typename same_endian_type, typename T>
 file_istream<same_endian_type>& operator >> ( file_istream<same_endian_type>& istm, T& val)
{
	istm.read(val);

	return istm;
}

template<typename same_endian_type>
 file_istream<same_endian_type>& operator >> ( file_istream<same_endian_type>& istm, std::string& val)
{
	val.clear();

	int size = 0;
	istm.read(size);

	if(size<=0)
		return istm;

	std::vector<char> vec((size_t)size);
	istm.read(vec);
	val.assign(&vec[0], (size_t)size);

	return istm;
}

template<typename same_endian_type>
class mem_istream
{
public:
	mem_istream() : m_index(0) {}
	mem_istream(const char * mem, size_t size) 
	{
		open(mem, size);
	}
	mem_istream(const std::vector<char>& vec) 
	{
		m_index = 0;
		m_vec.reserve(vec.size());
		m_vec.assign(vec.begin(), vec.end());
	}
	void open(const char * mem, size_t size)
	{
		m_index = 0;
		m_vec.clear();
		m_vec.reserve(size);
		m_vec.assign(mem, mem + size);
	}
	void close()
	{
		m_vec.clear();
	}
	bool eof() const
	{
		return m_index >= m_vec.size();
	}
	std::ifstream::pos_type tellg() const
	{
		return m_index;
	}
	bool seekg (size_t pos)
	{
		if(pos<m_vec.size())
			m_index = pos;
		else 
			return false;

		return true;
	}
	bool seekg (std::streamoff offset, std::ios_base::seekdir way)
	{
		if(way==std::ios_base::beg && offset < m_vec.size())
			m_index = offset;
		else if(way==std::ios_base::cur && (m_index + offset) < m_vec.size())
			m_index += offset;
		else if(way==std::ios_base::end && (m_vec.size() + offset) < m_vec.size())
			m_index = m_vec.size() + offset;
		else
			return false;

		return true;
	}

	const std::vector<char>& get_internal_vec()
	{
		return m_vec;
	}

	template<typename T>
	void read(T& t)
	{
		if(eof())
			throw std::runtime_error("Premature end of array!");

		if((m_index + sizeof(T)) > m_vec.size())
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&t), &m_vec[m_index], sizeof(T));

		nczarr::swap_endian_if_same_endian_is_false(t, m_same_type);

		m_index += sizeof(T);
	}

	void read(typename std::vector<char>& vec)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + vec.size()) > m_vec.size())
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&vec[0]), &m_vec[m_index], vec.size());

		m_index += vec.size();
	}

	void read(char* p, size_t size)
	{
		if(eof())
			throw std::runtime_error("Premature end of array!");

		if((m_index + size) > m_vec.size())
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(p), &m_vec[m_index], size);

		m_index += size;
	}

	void read(std::string& str, const unsigned int size)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + size) > m_vec.size())
			throw std::runtime_error("Premature end of array!");

		str.assign(&m_vec[m_index], size);

		m_index += str.size();
	}

private:
	std::vector<char> m_vec;
	size_t m_index;
	same_endian_type m_same_type;
};

template<typename same_endian_type, typename T>
 mem_istream<same_endian_type>& operator >> ( mem_istream<same_endian_type>& istm, T& val)
{
	istm.read(val);

	return istm;
}

template<typename same_endian_type>
mem_istream<same_endian_type>& operator >> (mem_istream<same_endian_type>& istm, std::string& val)
{
	val.clear();

	int size = 0;
	istm.read(size);

	if(size<=0)
		return istm;

	istm.read(val, size);

	return istm;
}

template<typename same_endian_type>
class ptr_istream
{
public:
	ptr_istream() : m_arr(nullptr), m_size(0), m_index(0) {}
	ptr_istream(const char * mem, size_t size) : m_arr(nullptr), m_size(0), m_index(0)
	{
		open(mem, size);
	}
	ptr_istream(const std::vector<char>& vec)
	{
		m_index = 0;
		m_arr = vec.data();
		m_size = vec.size();
	}
	void open(const char * mem, size_t size)
	{
		m_index = 0;
		m_arr = mem;
		m_size = size;
	}
	void close()
	{
		m_arr = nullptr; m_size = 0; m_index = 0;
	}
	bool eof() const
	{
		return m_index >= m_size;
	}
	std::ifstream::pos_type tellg() const
	{
		return m_index;
	}
	bool seekg(size_t pos)
	{
		if (pos<m_size)
			m_index = pos;
		else
			return false;

		return true;
	}
	bool seekg(std::streamoff offset, std::ios_base::seekdir way)
	{
		if (way == std::ios_base::beg && offset < m_size)
			m_index = offset;
		else if (way == std::ios_base::cur && (m_index + offset) < m_size)
			m_index += offset;
		else if (way == std::ios_base::end && (m_size + offset) < m_size)
			m_index = m_size + offset;
		else
			return false;

		return true;
	}

	template<typename T>
	void read(T& t)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + sizeof(T)) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&t), &m_arr[m_index], sizeof(T));

		nczarr::swap_endian_if_same_endian_is_false(t, m_same_type);

		m_index += sizeof(T);
	}

	void read(typename std::vector<char>& vec)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + vec.size()) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&vec[0]), &m_arr[m_index], vec.size());

		m_index += vec.size();
	}

	void read(char* p, size_t size)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + size) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(p), &m_arr[m_index], size);

		m_index += size;
	}

	void read(std::string& str, const unsigned int size)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + size) > m_size)
			throw std::runtime_error("Premature end of array!");

		str.assign(&m_arr[m_index], size);

		m_index += str.size();
	}

private:
	const char* m_arr;
	size_t m_size;
	size_t m_index;
	same_endian_type m_same_type;
};


template<typename same_endian_type, typename T>
 ptr_istream<same_endian_type>& operator >> ( ptr_istream<same_endian_type>& istm, T& val)
{
	istm.read(val);

	return istm;
}

template<typename same_endian_type>
 ptr_istream<same_endian_type>& operator >> ( ptr_istream<same_endian_type>& istm, std::string& val)
{
	val.clear();
	
	int size = 0;
	istm.read(size);

	if (size <= 0)
		return istm;

	istm.read(val, size);

	return istm;
}

template<typename same_endian_type>
class memfile_istream
{
public:
	memfile_istream() : m_arr(nullptr), m_size(0), m_index(0) {}
	memfile_istream(const char * file) : m_arr(nullptr), m_size(0), m_index(0)
	{
		open(file);
	}
#ifdef _MSC_VER
	memfile_istream(const wchar_t * file) : m_arr(nullptr), m_size(0), m_index(0)
	{
		open(file);
	}
#endif
	~memfile_istream()
	{
		close();
	}
	void open(const char * file)
	{
		close();
#ifdef _MSC_VER
		std::FILE* input_file_ptr = nullptr;
		fopen_s(&input_file_ptr, file, "rb");
#else
		std::FILE* input_file_ptr = std::fopen(file, "rb");
#endif
		compute_length(input_file_ptr);
		m_arr = new char[m_size];
		std::fread(m_arr, m_size, 1, input_file_ptr);
		fclose(input_file_ptr);
	}
#ifdef _MSC_VER
	void open(const wchar_t * file)
	{
		close();
		std::FILE* input_file_ptr = nullptr;
		_wfopen_s(&input_file_ptr, file, L"rb");
		compute_length(input_file_ptr);
		m_arr = new char[m_size];
		std::fread(m_arr, m_size, 1, input_file_ptr);
		fclose(input_file_ptr);
	}
#endif
	void close()
	{
		if (m_arr)
		{
			delete[] m_arr;
			m_arr = nullptr; m_size = 0; m_index = 0;
		}
	}
	bool is_open()
	{
		return (m_arr != nullptr);
	}
	long file_length() const
	{
		return m_size;
	}
	bool eof() const
	{
		return m_index >= m_size;
	}
	std::ifstream::pos_type tellg() const
	{
		return m_index;
	}
	bool seekg(size_t pos)
	{
		if (pos < m_size)
			m_index = pos;
		else
			return false;

		return true;
	}
	bool seekg(std::streamoff offset, std::ios_base::seekdir way)
	{
		if (way == std::ios_base::beg && offset < m_size)
			m_index = offset;
		else if (way == std::ios_base::cur && (m_index + offset) < m_size)
			m_index += offset;
		else if (way == std::ios_base::end && (m_size + offset) < m_size)
			m_index = m_size + offset;
		else
			return false;

		return true;
	}

	template<typename T>
	void read(T& t)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + sizeof(T)) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&t), &m_arr[m_index], sizeof(T));

		nczarr::swap_endian_if_same_endian_is_false(t, m_same_type);

		m_index += sizeof(T);
	}

	void read(typename std::vector<char>& vec)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + vec.size()) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(&vec[0]), &m_arr[m_index], vec.size());

		m_index += vec.size();
	}

	void read(char* p, size_t size)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + size) > m_size)
			throw std::runtime_error("Premature end of array!");

		std::memcpy(reinterpret_cast<void*>(p), &m_arr[m_index], size);

		m_index += size;
	}

	void read(std::string& str, const unsigned int size)
	{
		if (eof())
			throw std::runtime_error("Premature end of array!");

		if ((m_index + size) > m_size)
			throw std::runtime_error("Premature end of array!");

		str.assign(&m_arr[m_index], size);

		m_index += str.size();
	}

private:
	void compute_length(std::FILE* input_file_ptr)
	{
		std::fseek(input_file_ptr, 0, SEEK_END);
		m_size = std::ftell(input_file_ptr);
		std::fseek(input_file_ptr, 0, SEEK_SET);
	}

	char* m_arr;
	size_t m_size;
	size_t m_index;
	same_endian_type m_same_type;
};


template<typename same_endian_type, typename T>
 memfile_istream<same_endian_type>& operator >> ( memfile_istream<same_endian_type>& istm, T& val)
{
	istm.read(val);

	return istm;
}

template<typename same_endian_type>
 memfile_istream<same_endian_type>& operator >> ( memfile_istream<same_endian_type>& istm, std::string& val)
{
	val.clear();

	int size = 0;
	istm.read(size);

	if (size <= 0)
		return istm;

	istm.read(val, size);

	return istm;
}

template<typename same_endian_type>
class file_ostream
{
public:
	file_ostream() : output_file_ptr(nullptr) {}
	file_ostream(const char * file) : output_file_ptr(nullptr)
	{
		open(file);
	}
#ifdef _MSC_VER
	file_ostream(const wchar_t * file) : output_file_ptr(nullptr)
	{
		open(file);
	}
#endif
	~file_ostream()
	{
		close();
	}
	void open(const char * file)
	{
		close();
#ifdef _MSC_VER
		output_file_ptr = nullptr;
		fopen_s(&output_file_ptr, file, "wb");
#else
		output_file_ptr = std::fopen(file, "wb");
#endif
	}
#ifdef _MSC_VER
	void open(const wchar_t * file)
	{
		close();
		output_file_ptr = nullptr;
		_wfopen_s(&output_file_ptr, file, L"wb");
	}
#endif
	void flush()
	{
		std::fflush(output_file_ptr);
	}
	void close()
	{
		if (output_file_ptr)
		{
			std::fclose(output_file_ptr);
			output_file_ptr = nullptr;
		}
	}
	bool is_open()
	{
		return output_file_ptr != nullptr;
	}
	template<typename T>
	void write(const T& t)
	{
		T t2 = t;
		nczarr::swap_endian_if_same_endian_is_false(t2, m_same_type);
		std::fwrite(reinterpret_cast<const void*>(&t2), sizeof(T), 1, output_file_ptr);
	}
	void write(const std::vector<char>& vec)
	{
		std::fwrite(reinterpret_cast<const void*>(&vec[0]), vec.size(), 1, output_file_ptr);
	}
	void write(const char* p, size_t size)
	{
		std::fwrite(reinterpret_cast<const void*>(p), size, 1, output_file_ptr);
	}

private:
	std::FILE* output_file_ptr;
	same_endian_type m_same_type;
};

template<typename same_endian_type, typename T>
file_ostream<same_endian_type>& operator << (file_ostream<same_endian_type>& ostm, const T& val)
{
	ostm.write(val);

	return ostm;
}

template<typename same_endian_type>
 file_ostream<same_endian_type>& operator << ( file_ostream<same_endian_type>& ostm, const std::string& val)
{
	int size = val.size();
	ostm.write(size);

	if(val.size()<=0)
		return ostm;

	ostm.write(val.c_str(), val.size());

	return ostm;
}

template<typename same_endian_type>
 file_ostream<same_endian_type>& operator << ( file_ostream<same_endian_type>& ostm, const char* val)
{
	int size = std::strlen(val);
	ostm.write(size);

	if(size<=0)
		return ostm;

	ostm.write(val, size);

	return ostm;
}

template<typename same_endian_type>
class mem_ostream
{
public:
	mem_ostream() {}
	void close()
	{
		m_vec.clear();
	}
	const std::vector<char>& get_internal_vec()
	{
		return m_vec;
	}
	template<typename T>
	void write(const T& t)
	{
		std::vector<char> vec(sizeof(T));
		T t2 = t;
		nczarr::swap_endian_if_same_endian_is_false(t2, m_same_type);
		std::memcpy(reinterpret_cast<void*>(&vec[0]), reinterpret_cast<const void*>(&t2), sizeof(T));
		write(vec);
	}
	void write(const std::vector<char>& vec)
	{
		m_vec.insert(m_vec.end(), vec.begin(), vec.end());
	}
	void write(const char* p, size_t size)
	{
		for(size_t i=0; i<size; ++i)
			m_vec.push_back(p[i]);
	}
	template<typename T>
	void writeat(size_t pos, const T& t)
	{
		std::vector<char> vec(sizeof(T));
		T t2 = t;
		nczarr::swap_endian_if_same_endian_is_false(t2, m_same_type);
		std::memcpy(reinterpret_cast<void*>(&vec[0]), reinterpret_cast<const void*>(&t2), sizeof(T));
		writeat(pos, vec);
	}

	void writeat(size_t pos, const std::vector<char>& vec)
	{
		for (size_t n = 0, count = vec.size(); n < count; n++)
			m_vec[pos++] = vec[n];
	}
private:
	std::vector<char> m_vec;
	same_endian_type m_same_type;
};

template<typename same_endian_type, typename T>
 mem_ostream<same_endian_type>& operator << ( mem_ostream<same_endian_type>& ostm, const T& val)
{
	ostm.write(val);

	return ostm;
}

template<typename same_endian_type>
 mem_ostream<same_endian_type>& operator << ( mem_ostream<same_endian_type>& ostm, const std::string& val)
{
	int size = val.size();
	ostm.write(size);

	if(val.size()<=0)
		return ostm;

	ostm.write(val.c_str(), val.size());

	return ostm;
}

template<typename same_endian_type>
 mem_ostream<same_endian_type>& operator << ( mem_ostream<same_endian_type>& ostm, const char* val)
{
	int size = std::strlen(val);
	ostm.write(size);

	if(size<=0)
		return ostm;

	ostm.write(val, size);

	return ostm;
}

template<typename same_endian_type>
class memfile_ostream
{
public:
	memfile_ostream() {}
	void close()
	{
		m_vec.clear();
	}
	const std::vector<char>& get_internal_vec()
	{
		return m_vec;
	}
	template<typename T>
	void write(const T& t)
	{
		std::vector<char> vec(sizeof(T));
		T t2 = t;
		nczarr::swap_endian_if_same_endian_is_false(t2, m_same_type);
		std::memcpy(reinterpret_cast<void*>(&vec[0]), reinterpret_cast<const void*>(&t2), sizeof(T));
		write(vec);
	}
	void write(const std::vector<char>& vec)
	{
		m_vec.insert(m_vec.end(), vec.begin(), vec.end());
	}
	void write(const char* p, size_t size)
	{
		for (size_t i = 0; i<size; ++i)
			m_vec.push_back(p[i]);
	}
	template<typename T>
	void writeat(size_t pos, const T& t)
	{
		std::vector<char> vec(sizeof(T));
		T t2 = t;
		nczarr::swap_endian_if_same_endian_is_false(t2, m_same_type);
		std::memcpy(reinterpret_cast<void*>(&vec[0]), reinterpret_cast<const void*>(&t2), sizeof(T));
		writeat(pos, vec);
	}

	void writeat(size_t pos, const std::vector<char>& vec)
	{
		for (size_t n = 0, count = vec.size(); n < count; n++)
			m_vec[pos++] = vec[n];
	}
	bool write_to_file(const char* file)
	{
#ifdef _MSC_VER
		std::FILE* fp = nullptr;
		fopen_s(&fp, file, "wb");
#else
		std::FILE* fp = std::fopen(file, "wb");
#endif
		if (fp)
		{
			size_t size = std::fwrite(m_vec.data(), m_vec.size(), 1, fp);
			std::fflush(fp);
			std::fclose(fp);
			m_vec.clear();
			return size == 1u;
		}
		return false;
	}
#ifdef _MSC_VER
	bool write_to_file(const wchar_t* file)
	{
		std::FILE* fp = nullptr;
		_wfopen_s(&fp, file, L"wb");
		if (fp)
		{
			size_t size = std::fwrite(m_vec.data(), m_vec.size(), 1, fp);
			std::fflush(fp);
			std::fclose(fp);
			m_vec.clear();
			return size == 1u;
		}
		return false;
	}
#endif

private:
	std::vector<char> m_vec;
	same_endian_type m_same_type;
};

template<typename same_endian_type, typename T>
 memfile_ostream<same_endian_type>& operator << ( memfile_ostream<same_endian_type>& ostm, const T& val)
{
	ostm.write(val);

	return ostm;
}

template<typename same_endian_type>
 memfile_ostream<same_endian_type>& operator << ( memfile_ostream<same_endian_type>& ostm, const std::string& val)
{
	int size = val.size();
	ostm.write(size);

	if (val.size() <= 0)
		return ostm;

	ostm.write(val.c_str(), val.size());

	return ostm;
}

template<typename same_endian_type>
 memfile_ostream<same_endian_type>& operator << ( memfile_ostream<same_endian_type>& ostm, const char* val)
{
	int size = std::strlen(val);
	ostm.write(size);

	if (size <= 0)
		return ostm;

	ostm.write(val, size);

	return ostm;
}

} // ns simple

#endif // SimpleBinStream_H
