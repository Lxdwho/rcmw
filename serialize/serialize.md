# Serialize
> 序列化的目的是将各种数据结构：int、float、string、vector、map等架构化的数据存放到一段连续的内存中，方便程序去做数据的传输与存储

在cmw中实现了一个简易的序列化类，能够将`bool、char、int32_t、uint32_t、int64_t、uint64_t、string、vector、map、set、list`等数据类型转为`chat`类型的数组进行存储。该类的声名如下：
```cpp
class DataStream
{
public:
    enum DataType
    {
        BOOL = 0,  
        CHAR,
        INT32,
        INT64,
        UINT32,
        UINT64,
        FLOAT,
        DOUBLE,
        ENUM,
        STRING,
        VECTOR,
        LIST,
        MAP,
        SET,
        CUSTOM
    };

    enum ByteOrder
    {
        BigEndian,
        LittleEndian
    };

    DataStream();
    DataStream(const string & data);
    DataStream(const char* ptr, size_t size);
    ~DataStream();

    void show() const;
    void write(const char * data, int len);
    void write(bool value);
    void write(char value);
    void write(int32_t value);
    void write(uint32_t value);
    void write(uint64_t value);
    void write(int64_t value);
    void write(float value);
    void write(double value);
    void write(const char * value);
    void write(const string & value);
    void write(const Serializable & value);
 
    template <typename T>
    void write(const std::vector<T> & value);

    template <typename T>
    void write(const std::list<T> & value);

    template <typename K, typename V>
    void write(const std::map<K, V> & value);

    template <typename T>
    void write(const std::set<T> & value);

    //采用SFINAE特性保证T为模板类型
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    void write(const T& value);

    template <typename T, typename ...Args>
    void write_args(const T & head, const Args&... args);

    void write_args();

    bool read(char * data, int len);
    bool read(bool & value);
    bool read(char & value);
    bool read(int32_t & value);
    bool read(uint32_t& value);
    bool read(uint64_t& value);
    bool read(int64_t & value);
    bool read(float & value);
    bool read(double & value);
    bool read(string & value);
    bool read(Serializable & value);

    template <typename T>
    bool read(std::vector<T> & value);

    template <typename T>
    bool read(std::list<T> & value);

    template <typename K, typename V>
    bool read(std::map<K, V> & value);

    template <typename T>
    bool read(std::set<T> & value);

    //采用SFINAE特性保证T为枚举类型
    template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
    bool read(T& value);

    template <typename T, typename ...Args>
    bool read_args(T & head, Args&... args);

    bool read_args();

    const char * data() const;
    int size() const;
    size_t ByteSize();
    void clear();
    void reset();
    void save(const string & filename);
    void load(const string & filename);

    DataStream & operator << (bool value);
    DataStream & operator << (char value);
    DataStream & operator << (int32_t value);
    DataStream & operator << (int64_t value);
    DataStream & operator << (uint32_t value);
    DataStream & operator << (uint64_t value);
    DataStream & operator << (float value);
    DataStream & operator << (double value);
    DataStream & operator << (const char * value);
    DataStream & operator << (const string & value);
    DataStream & operator << (const Serializable & value);

    template <typename T>
    DataStream & operator << (const std::vector<T> & value);

    template <typename T>
    DataStream & operator << (const std::list<T> & value);

    template <typename K, typename V>
    DataStream & operator << (const std::map<K, V> & value);

    template <typename T>
    DataStream & operator << (const std::set<T> & value);

    DataStream & operator >> (bool & value);
    DataStream & operator >> (char & value);
    DataStream & operator >> (int32_t & value);
    DataStream & operator >> (int64_t & value);
    DataStream & operator >> (uint32_t & value);
    DataStream & operator >> (uint64_t & value);
    DataStream & operator >> (float & value);
    DataStream & operator >> (double & value);
    DataStream & operator >> (string & value);
    DataStream & operator >> (Serializable & value);

    template <typename T>
    DataStream & operator >> (std::vector<T> & value);

    template <typename T>
    DataStream & operator >> (std::list<T> & value);

    template <typename K, typename V>
    DataStream & operator >> (std::map<K, V> & value);

    template <typename T>
    DataStream & operator >> (std::set<T> & value);

private:
    void reserve(int len);
    ByteOrder byteorder();

private:
    std::vector<char> m_buf;
    int m_pos;
    ByteOrder m_byteorder;
};
```
`DataStream`类内维护了3个变量：
- `m_buf`：char类型的vector用于存放序列化后的数据
- `m_pos`：m_buf的反序列化索引
- `m_byteorder`：序列化的大小端标志
类内部提供了一些列的write、read操作，用于对数据进行序列化以及反序列化操作
## 1 序列化操作
`Datastream`的序列化操作由`write`函数实现，在`Datastream`中重载了许多`write`函数，用于对不同类型的数据进行序列化，具体如下：
```cpp
void write(const char * data, int len);
void write(bool value);
void write(char value);
void write(int32_t value);
void write(uint32_t value);
void write(uint64_t value);
void write(int64_t value);
void write(float value);
void write(double value);
void write(const char * value);
void write(const string & value);
void write(const Serializable & value);

template <typename T>
void write(const std::vector<T> & value);

template <typename T>
void write(const std::list<T> & value);

template <typename K, typename V>
void write(const std::map<K, V> & value);

template <typename T>
void write(const std::set<T> & value);

//采用SFINAE特性保证T为模板类型
template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
void write(const T& value);

template <typename T, typename ...Args>
void write_args(const T & head, const Args&... args);

void write_args();
```
在上面的一系列`write`函数中，函数`void write(const char * data, int len);`负责将数据切实地写入到`m_buf`中进行序列化，其他的`write`函数通过调用该函数实现序列化
```cpp
void DataStream::write(const char * data, int len)
{
    reserve(len);
    int size = m_buf.size();
    m_buf.resize(m_buf.size() + len);
    std::memcpy(&m_buf[size], data, len);
}
```
如函数`void write(char value);`以及`void write(uint64_t value);`
```cpp
void DataStream::write(char value)
{
    char type = DataType::CHAR;
    write((char *)&type, sizeof(char));
    write((char *)&value, sizeof(char));
}

void DataStream::write(uint64_t value)
{
    char type = DataType::UINT64;
    write((char*)&type ,sizeof(char));
    if(m_byteorder == ByteOrder::BigEndian)
    {
        char * first = (char*)&value;
        char * last = first + sizeof(uint64_t);
        std::reverse(first, last);
    }
    write((char*)&value, sizeof(uint64_t));
}
```
对于单字节的数据类型，直接进行写入即可，但是对于多字节的数据类型，需要根据数据存储的大小端模式进行转换（元素级别的反转？）

## n 下不下去了后边写
主要就是一个参数模板、宏的使用以及对vector的理解，自行理解体会

