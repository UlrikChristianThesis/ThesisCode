#pragma once 
#include<vector>
#include<algorithm>
#include<cassert>
#include<iostream>
#include<fstream>
#include<iomanip>

template <typename T>
class Matrix
{
private:
    std::vector<T>  data;
    size_t          rows;
    size_t          cols;
public:
    // CTORS
    Matrix(): rows(0), cols(0) {}
    Matrix(const size_t nrows, const size_t ncols): rows(nrows), cols(ncols), data(nrows * ncols){}

    //Ville selv bruge getters, men Ant' klarer det uden? Venner siden begge er Mtraix?  
    Matrix(const Matrix& M): rows(M.rows), cols(M.cols), data(M.data){}

    // Matrix(std::string file_path, size_t dim) : rows(dim), cols(dim), data(dim * dim)
    Matrix(std::string file_path, size_t n, size_t m)
    {
        Matrix<T> res(n, m);
        std::ifstream myRawMatrix (file_path);
        std::string entry;

        for(size_t i = 0; i < n ; ++i)
        {
            for(size_t j = 0; j < m; ++j)
            {
                std::getline(myRawMatrix, entry, ',');
                res[i][j] = std::stod(entry);
            }
        }
        
        myRawMatrix.close();
        *this = res;
    }

    Matrix& operator=(Matrix& M)
    {
        if(this == &M) return *this;
        Matrix cpy(M);
        grab_members(cpy);
        return *this;
    }

    // Alt. CTOR + assign (Ant)
    template<typename U>
    Matrix(const Matrix<U>& M)
    {
        rows = M.get_rows();
        cols = M.get_cols();
        data.resize(rows*cols);
        std::copy(M.begin(), M.end(), data.begin());
    }

    template<typename U>
    Matrix& operator=(const Matrix<U>& M)
    {
        if (this == &M) return *this;
        Matrix<T> cpy(M);   
        grab_members(cpy);
        return *this;
    }

    // Data fillers 
    void fill(const T& val)
    {
        std::fill(begin(), end(), val);
    }

    void fill(std::vector<T>& values)
    {
        assert(values.size == data.size());
        std::transform(values.begin(), values.end(), begin(),
        [](const T& val){return val;});
    }

    void resizeM(size_t nrows, size_t ncols)
    {
        // Set dimensions
        rows = nrows;
        cols = ncols;

        // Check storage
        if (data.size() > nrows*ncols) return;
         
        data.resize(nrows*ncols);
    }

    void grab_members(Matrix& M)
    {
        data.swap(M.data);
        // std::swap(rows, M.get_rows());
        std::swap(rows, M.rows);
        // std::swap(cols, M.get_cols());
        std::swap(cols, M.cols);
    }

    template<typename lambda>
    void transform(lambda lambda_func)
    {
        std::transform(data.begin(), data.end(), data.begin(), lambda_func);
    }

    size_t get_rows() const {return rows;}
    size_t get_cols() const {return cols;}
    
    typedef typename std::vector<T>::iterator       matrix_iterator;
    typedef typename std::vector<T>::const_iterator c_matrix_iterator;

    matrix_iterator begin() {return data.begin();}
    matrix_iterator end()   {return data.end();}
    c_matrix_iterator begin() const {return data.begin();}
    c_matrix_iterator end() const   {return data.end();}

    T* operator[] (const size_t row) {return &data[row * cols];}
    const T* operator[] (const size_t row) const {return &data[row * cols];}

    Matrix operator+() {return *this;}
    Matrix operator-()
    {
        std::transform(begin(), end(), begin(), [](const T& el){return 0.0 - el;});
        return *this;
    }

    Matrix operator+=(const Matrix<T>& A)
    {
        assert(match_dim(*this, A));
        std::transform(A.begin(), A.end(), begin(), begin(),
        [](const T& a, const T& b){return a + b;});
    }

    Matrix operator-=(const Matrix<T>& A)
    {
        assert(match_dim(*this, A));
        std::transform(A.begin(), A.end(), begin(), begin(),
        [](const T& a, const T& b){return a - b;});
    }

    Matrix operator*=(const Matrix<T>& A)
    {
        assert(match_dim(*this, A));
        Matrix<T> temp = matrix_product_naive(*this, A);
        grab_members(temp);

        return *this;
    }

    Matrix inline friend  operator+(const Matrix<T>& A, const Matrix<T>& B)
    {
        assert(match_dim(A, B));
        Matrix<T> res(A.get_rows(), A.get_cols());
        std::transform(A.begin(), A.end(), B.begin(), res.begin(),
        [](const T& a, const T& b){return a + b;});
        return res;
    }

    Matrix inline friend  operator-(const Matrix<T>& A, const Matrix<T>& B)
    {
        assert(match_dim(A, B));
        Matrix<T> res(A.get_rows(), A.get_cols());
        std::transform(A.begin(), A.end(), B.begin(), res.begin(),
        [](const T& a, const T& b){return a - b;});
        return res;
    }

    Matrix inline friend  operator*(const Matrix<T>& A, const Matrix<T>& B)
    {
        assert(match_dim(A, B));
        Matrix<T> res = matrix_product_naive(A, B);
        return res;
    }
};

template<typename T>
inline Matrix<T> transpose(Matrix<T>& M)
{
    size_t rows = M.get_rows();
    size_t cols = M.get_cols(); 

    Matrix<T> res(cols, rows);
    for (size_t i = 0; i < cols; ++i)
    {
        for (size_t j = 0; j < rows; ++j)
        {
            res[i][j] = M[j][i];
        }
    }
    return res;
}

template<typename T>
bool match_dim(const Matrix<T>& A, const Matrix<T>& B)
{
    return (A.get_rows() == B.get_rows() || A.get_cols() == B.get_cols());
}

template<typename T>
Matrix<T> matrix_product_naive(
            const Matrix<T>& X, 
            const Matrix<T>& Y)
{
    const size_t rows = X.get_rows(); 
    const size_t temp = X.get_cols(); 
    const size_t cols = Y.get_cols(); // y_rows = Y.rows(),
 
    Matrix<T> Z(rows, cols);
    // Matrix<double> res(rows, cols);
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            T tmp = 0;
            for (size_t k = 0; k < temp; ++k)
            {
                tmp += X[i][k] * Y[k][j];               
            }
            Z[i][j] = tmp;            
        }    
    }
    return Z;
}
