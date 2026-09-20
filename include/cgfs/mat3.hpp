#ifndef CGFS_MAT3_HPP
#define CGFS_MAT3_HPP

#include <array>
#include <cassert>
#include <concepts>
#include <span>
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/**
 * @brief Represents a 3x3 matrix.
 *
 * @tparam T The type of the elements in the matrix.
 */
template<std::floating_point T>
class Mat3 final {
public:

    /** @brief The number of rows in this matrix. */
    static constexpr isize ROWS = 3;

    /** @brief The number of columns in this matrix. */
    static constexpr isize COLS = 3;

private:

    /** @brief The elements of this matrix. */
    std::array<T, ROWS * COLS> _elems;

public:

    /** @brief Initializes a matrix with all elements set to zero. */
    constexpr Mat3() noexcept : _elems({ }) { }

    /**
     * @brief Initializes a matrix with the specified elements.
     *
     * @note The elements must be specified in row-major order.
     *
     * @param[in] e00 The element at row `0` and column `0`.
     * @param[in] e01 The element at row `0` and column `1`.
     * @param[in] e02 The element at row `0` and column `2`.
     * @param[in] e10 The element at row `1` and column `0`.
     * @param[in] e11 The element at row `1` and column `1`.
     * @param[in] e12 The element at row `1` and column `2`.
     * @param[in] e20 The element at row `2` and column `0`.
     * @param[in] e21 The element at row `2` and column `1`.
     * @param[in] e22 The element at row `2` and column `2`.
     */
    constexpr Mat3(
        T e00,
        T e01,
        T e02,
        T e10,
        T e11,
        T e12,
        T e20,
        T e21,
        T e22
    ) noexcept
        : _elems({ e00, e01, e02, e10, e11, e12, e20, e21, e22 }) { }

    /**
     * @brief Initializes a matrix with the specified elements.
     *
     * @note The elements must be specified in row-major order.
     *
     * @param[in] elems The elements of the matrix.
     */
    constexpr Mat3(std::span<const T, ROWS * COLS> elems) noexcept
        : _elems(elems) { }

    /**
     * @brief Returns the identity matrix.
     *
     * @return The identity matrix.
     */
    static constexpr Mat3<T> identity() noexcept {
        return {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        };
    }

    /**
     * @brief Returns a row of this matrix.
     *
     * @param[in] row The index of the row to retrieve.
     * @return The specified row of this matrix.
     */
    constexpr std::span<const T, COLS> operator[](isize row) const noexcept {
        assert((row >= 0) && (row < ROWS));
        return std::span<const T, COLS>(_elems.data() + row * COLS, COLS);
    }

    /**
     * @brief Returns a row of this matrix.
     *
     * @param[in] row The index of the row to retrieve.
     * @return The specified row of this matrix.
     */
    constexpr std::span<T, COLS> operator[](isize row) noexcept {
        assert((row >= 0) && (row < ROWS));
        return std::span<T, COLS>(_elems.data() + row * COLS, COLS);
    }

    /**
     * @brief Multiplies this matrix by a scalar.
     *
     * @param[in] rhs The scalar to multiply by.
     * @return A reference to this matrix after the multiplication.
     */
    constexpr Mat3<T>& operator*=(T rhs) noexcept {
        for (isize i = 0; i < ROWS; i++) {
            for (isize j = 0; j < COLS; j++) {
                (*this)[i][j] *= rhs;
            }
        }
        return *this;
    }

    /**
     * @brief Multiplies a matrix by a scalar.
     *
     * @param[in] lhs The matrix to multiply.
     * @param[in] rhs The scalar to multiply by.
     * @return The product of the matrix and the scalar.
     */
    friend constexpr Mat3<T> operator*(Mat3<T> lhs, T rhs) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Multiplies a matrix by a scalar.
     *
     * @param[in] lhs The scalar to multiply by.
     * @param[in] rhs The matrix to multiply.
     * @return The product of the matrix and the scalar.
     */
    friend constexpr Mat3<T> operator*(T lhs, Mat3<T> rhs) noexcept {
        rhs *= lhs;
        return rhs;
    }

    /**
     * @brief Multiplies a matrix by a vector.
     *
     * @param[in] lhs The matrix to multiply.
     * @param[in] rhs The vector to multiply by.
     * @return The product of the matrix and the vector.
     */
    friend constexpr Vec3<T> operator*(
        const Mat3<T>& lhs,
        const Vec3<T>& rhs
    ) noexcept {
        return {
            lhs[0][0] * rhs.x + lhs[0][1] * rhs.y + lhs[0][2] * rhs.z,
            lhs[1][0] * rhs.x + lhs[1][1] * rhs.y + lhs[1][2] * rhs.z,
            lhs[2][0] * rhs.x + lhs[2][1] * rhs.y + lhs[2][2] * rhs.z
        };
    }

    /**
     * @brief Multiplies this matrix by another matrix.
     *
     * @param[in] rhs The matrix to multiply by.
     * @return A reference to this matrix after the multiplication.
     */
    constexpr Mat3<T>& operator*=(const Mat3<T>& rhs) noexcept {
        Mat3<T> result;
        for (isize i = 0; i < ROWS; i++) {
            for (isize j = 0; j < COLS; j++) {
                for (isize k = 0; k < COLS; k++) {
                    result[i][j] += (*this)[i][k] * rhs[k][j];
                }
            }
        }
        *this = result;
        return *this;
    }

    /**
     * @brief Multiplies two matrices.
     *
     * @param[in] lhs The first matrix.
     * @param[in] rhs The second matrix.
     * @return The product of the two matrices.
     */
    friend constexpr Mat3<T> operator*(
        Mat3<T> lhs,
        const Mat3<T>& rhs
    ) noexcept {
        lhs *= rhs;
        return lhs;
    }

    /**
     * @brief Returns a span containing the elements of this matrix.
     *
     * @return A span containing the elements of this matrix.
     */
    constexpr std::span<const T, ROWS * COLS> elems() const noexcept {
        return std::span<const T, ROWS * COLS>(_elems.data(), ROWS * COLS);
    }

    /**
     * @brief Returns a span containing the elements of this matrix.
     *
     * @return A span containing the elements of this matrix.
     */
    constexpr std::span<T, ROWS * COLS> elems() noexcept {
        return std::span<T, ROWS * COLS>(_elems.data(), ROWS * COLS);
    }

    /**
     * @brief Returns the transpose of this matrix.
     *
     * @return The transpose of this matrix.
     */
    constexpr Mat3<T> transpose() const noexcept {
        Mat3<T> result;
        for (isize i = 0; i < ROWS; i++) {
            for (isize j = 0; j < COLS; j++) {
                result[i][j] = (*this)[j][i];
            }
        }
        return result;
    }

    /**
     * @brief Returns the determinant of this matrix.
     *
     * @return The determinant of this matrix.
     */
    constexpr T determinant() const noexcept {
        const Mat3<T>& m = *this;
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
            - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
            + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    /**
     * @brief Returns the inverse of this matrix.
     *
     * @warning The behavior is undefined if the matrix is not invertible (i.e.,
     * its determinant is zero).
     *
     * @return The inverse of this matrix.
     */
    constexpr Mat3<T> inverse() const noexcept {
        T det = determinant();
        T invDet = static_cast<T>(1) / det;
        const Mat3<T>& m = *this;
        return {
            invDet * (m[1][1] * m[2][2] - m[1][2] * m[2][1]),
            invDet * (m[0][2] * m[2][1] - m[0][1] * m[2][2]),
            invDet * (m[0][1] * m[1][2] - m[0][2] * m[1][1]),
            invDet * (m[1][2] * m[2][0] - m[1][0] * m[2][2]),
            invDet * (m[0][0] * m[2][2] - m[0][2] * m[2][0]),
            invDet * (m[0][2] * m[1][0] - m[0][0] * m[1][2]),
            invDet * (m[1][0] * m[2][1] - m[1][1] * m[2][0]),
            invDet * (m[0][1] * m[2][0] - m[0][0] * m[2][1]),
            invDet * (m[0][0] * m[1][1] - m[0][1] * m[1][0])
        };
    }

    /**
     * @brief Returns the trace of this matrix.
     *
     * @note The trace of a matrix is the sum of the elements on its main
     * diagonal (from the top-left to the bottom-right).
     *
     * @return The trace of this matrix.
     */
    constexpr T trace() const noexcept {
        return (*this)[0][0] + (*this)[1][1] + (*this)[2][2];
    }

};

}

#endif