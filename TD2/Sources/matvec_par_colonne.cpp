// Produit matrice-vecteur
# include <cassert>
# include <vector>
# include <iostream>
# include <cstdlib>
# include <string>
# include <chrono>
# include <cmath>
# include <fstream>
# include <mpi.h>
#include <algorithm>


// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix(int dim);
    Matrix(int nrows, int ncols);
    Matrix(const Matrix& A) = delete;
    Matrix(Matrix&& A) = default;
    ~Matrix() = default;

    Matrix& operator = (const Matrix& A) = delete;
    Matrix& operator = (Matrix&& A) = default;

    double& operator () (int i, int j) {
        return m_arr_coefs[i + j * m_nrows];
    }
    double  operator () (int i, int j) const {
        return m_arr_coefs[i + j * m_nrows];
    }

    std::vector<double> operator * (const std::vector<double>& u) const;

    std::ostream& print(std::ostream& out) const
    {
        const Matrix& A = *this;
        out << "[\n";
        for (int i = 0; i < m_nrows; ++i) {
            out << " [ ";
            for (int j = 0; j < m_ncols; ++j) {
                out << A(i, j) << " ";
            }
            out << " ]\n";
        }
        out << "]";
        return out;
    }
private:
    int m_nrows, m_ncols;
    std::vector<double> m_arr_coefs;
};
// ---------------------------------------------------------------------
inline std::ostream&
operator << (std::ostream& out, const Matrix& A)
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream&
operator << (std::ostream& out, const std::vector<double>& u)
{
    out << "[ ";
    for (const auto& x : u)
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double>
Matrix::operator * (const std::vector<double>& u) const
{
    const Matrix& A = *this;
    assert(u.size() == unsigned(m_ncols));
    std::vector<double> v(m_nrows, 0.);
    for (int i = 0; i < m_nrows; ++i) {
        for (int j = 0; j < m_ncols; ++j) {
            v[i] += A(i, j) * u[j];
        }
    }
    return v;
}

// =====================================================================
Matrix::Matrix(int dim) : m_nrows(dim), m_ncols(dim),
m_arr_coefs(dim* dim)
{
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            (*this)(i, j) = (i + j) % dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix(int nrows, int ncols) : m_nrows(nrows), m_ncols(ncols),
m_arr_coefs(nrows* ncols)
{
    int dim = (nrows > ncols ? nrows : ncols);
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            (*this)(i, j) = (i + j) % dim;
        }
    }
}
// =====================================================================
int main(int nargs, char* argv[]){


    MPI_Init(&nargs, &argv);
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

    int nbp;
    MPI_Comm_size(globComm, &nbp);

    int rank;
    MPI_Comm_rank(globComm, &rank);

    MPI_Status status;
    int tag = 1111;


    const int N = 6;

    const int N_loc = N / nbp;

    
    if (rank == 0) {

        Matrix A(N);


        Matrix A_loc(N,N_loc);
        std::vector<double> u_loc(N_loc);
        for (int i = 0; i < N_loc; ++i) u_loc[i] = i + 1 + 2 * rank;
        std::vector<double> v = A_loc * u_loc;
        //std::cout << "A.u loc = " << v << std::endl;

        std::vector<double> v_recv(N);
        for (int i = 0; i < N_loc; i++) {
            MPI_Recv(v_recv.data(), N, MPI_DOUBLE, MPI_ANY_SOURCE, tag, globComm, &status);
            std::transform(v.begin(), v.end(), v_recv.begin(), v.begin(), std::plus<double>());
        }
        std::cout << "A.u = " << v << std::endl;
        
        MPI_Bcast(v.data(), 2*N, MPI_INT, 0, globComm);


    }
    else {

        Matrix A_loc(N, N_loc);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N_loc; j++)
                A_loc(i, j) = (i + j + N_loc*rank) % N;
        }
        
        std::vector<double> u_loc(N_loc);
        for (int i = 0; i < N_loc; ++i) u_loc[i] = i + 1 + 2*rank;
        std::vector<double> v_loc = A_loc * u_loc;
        //std::cout << "A.u loc = " << v_loc << std::endl;
        MPI_Send(v_loc.data(), N, MPI_DOUBLE, 0, tag, globComm);

        MPI_Bcast(v_loc.data(), 2*N, MPI_DOUBLE, 0, globComm);//pourquoi avec 2N marche, et N marche pas?
        std::cout << "Processus " << rank << " a recu le resultat:= "<<v_loc<<"\n";

    }


    MPI_Finalize();

    return EXIT_SUCCESS;
}
