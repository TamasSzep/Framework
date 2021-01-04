// MathHelper.cpp

#include "MathHelper.h"

#include <stdexcept>
#include <cstring>
#include <cmath>

using namespace Core;

void MathHelper::SolveLinearEquationSystem(const std::vector<std::vector<float>>& A,
	const std::vector<float>& b, std::vector<float>& result)
{
	// Getting dimensions.
	int n = (int)A.size();
	if ((int)A[0].size() != n || (int)b.size() != n)
	{
		throw std::runtime_error("Linear equation doesn't have a quadratic A matrix.");
	}

	// Copying matrix and vector.
	std::vector<std::vector<float>> nA;
	std::vector<float> nb;
	nA.resize(n);
	nb.resize(n);
	memcpy(&nb[0], &b[0], n);
	for (int i = 0; i < n; i++)
	{
		nA[i].resize(n);
		memcpy(&nA[i][0], &A[i][0], n);
	}

	const float epsilon = 1e-6f;

	// Forward Gauss elimination.
	for (int i = 0; i < n; i++)
	{
		if (fabsf(nA[i][i]) <= epsilon)
		{
			if (i == n - 1)
			{
				if (fabsf(nb[n - 1]) <= epsilon)
				{
					throw std::runtime_error("Infinite solutions.");
				}
				else
				{
					throw std::runtime_error("No solutions.");
				}
			}

			// Trying to permute rows.
			int j = i + 1;
			for (; j < n; j++)
			{
				if (fabsf(nA[j][i]) > epsilon)
				{
					break;
				}
			}
			if (j == n)
			{
				throw std::runtime_error("Gauss elimination case not implemented.");
			}
			else
			{
				std::vector<float> temp = nA[i];
				nA[i] = nA[j];
				nA[j] = temp;
			}
		}
		float a = nA[i][i];
		nA[i][i] = 1;
		for (int j = i + 1; j < n; j++)
		{
			nA[i][j] /= a;
		}
		nb[i] /= a;
		for (int j = i + 1; j < n; j++)
		{
			float x = nA[j][i];
			nA[j][i] = 0;
			for (int k = i + 1; k < n; k++)
			{
				nA[j][k] -= x * nA[i][k];
			}
			nb[j] -= x * nb[i];
		}
	}

	// Backward Gauss elimination.
	for (int i = n - 1; i >= 0; i--)
	{
		for (int j = i - 1; j >= 0; j--)
		{
			float x = nA[j][i];
			nA[j][i] = 0;
			for (int k = i - 1; k > j; k--)
			{
				nA[j][k] -= x * nA[i][k];
			}
			nb[j] -= x * nb[i];
		}
	}

	result = nb;
}
