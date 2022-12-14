#pragma once

namespace librapid { namespace internal {
	template<typename ArrT>
	class CommaInitializer {
	public:
		using Scalar = typename traits<ArrT>::Scalar;

		CommaInitializer() = delete;
		explicit CommaInitializer(ArrT &dst, const Scalar &val) : m_array(dst) {
			next(val);
		}

		CommaInitializer &operator,(const Scalar &other) {
			next(other);
			return *this;
		}

		void next(const Scalar &other) {
			m_array.storage()[m_index] = other;
			++m_index;
		}

	private:
		ArrT &m_array;
		i64 m_index = 0;
	};
} } // namespace librapid::internal