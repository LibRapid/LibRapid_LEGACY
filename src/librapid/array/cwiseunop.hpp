#pragma once

namespace librapid {
	namespace internal {
		template<typename Unop, typename TYPE>
		struct traits<unop::CWiseUnop<Unop, TYPE>> {
			static constexpr bool IsScalar	  = false;
			static constexpr bool IsEvaluated = false;
			using Valid						  = std::true_type;
			using Type						  = unop::CWiseUnop<Unop, TYPE>;
			using Scalar					  = typename Unop::RetType;
			using BaseScalar				  = typename traits<Scalar>::BaseScalar;
			using Packet					  = typename traits<Scalar>::Packet;
			using Device					  = typename traits<TYPE>::Device;
			using StorageType				  = memory::DenseStorage<Scalar, Device>;
			static constexpr ui64 Flags = (Unop::Flags | traits<TYPE>::Flags) & ~flags::Evaluated;
		};
	} // namespace internal

	namespace unop {
		template<typename Unop, typename TYPE>
		class CWiseUnop
				: public ArrayBase<CWiseUnop<Unop, TYPE>, typename internal::traits<TYPE>::Device> {
		public:
			using Operation				= Unop;
			using Scalar				= typename Unop::RetType;
			using Packet				= typename internal::traits<Scalar>::Packet;
			using ValType				= typename internal::StripQualifiers<TYPE>;
			using Device				= typename internal::traits<TYPE>::Device;
			using Type					= CWiseUnop<Unop, TYPE>;
			using Base					= ArrayBase<Type, Device>;
			static constexpr ui64 Flags = internal::traits<Type>::Flags;

			CWiseUnop() = delete;

			template<typename... Args>
			explicit CWiseUnop(const ValType &value, Args... opArgs) :
					Base(value.extent(), 0), m_value(value), m_operation(opArgs...) {}

			CWiseUnop(const Type &op) :
					Base(op.extent(), 0), m_value(op.m_value), m_operation(op.m_operation) {}

			template<typename T>
			CWiseUnop &operator=(const T &op) = delete;

			LR_NODISCARD("") Array<Scalar, Device> operator[](i64 index) const {
				LR_WARN_ONCE(
				  "Calling operator[] on a lazy-evaluation object forces evaluation every time. "
				  "Consider using operator() instead");

				auto res = eval();
				return res[index];
			}

			template<typename... T>
			LR_NODISCARD("")
			auto operator()(T... indices) const {
				LR_ASSERT((this->m_isScalar && sizeof...(T) == 1) ||
							sizeof...(T) == Base::extent().ndim(),
						  "Array with {0} dimensions requires {0} access indices. Received {1}",
						  Base::extent().ndim(),
						  sizeof...(indices));

				i64 index = Base::isScalar() ? 0 : Base::extent().index(indices...);
				return scalar(index);
			}

			LR_NODISCARD("Do not ignore the result of an evaluated calculation")
			Array<Scalar, Device> eval() const {
				Array<Scalar, Device> res(m_operation.genExtent(m_value.extent()));

				if constexpr ((bool)(Flags & internal::flags::HasCustomEval)) {
					m_operation.customEval(m_value, res);
					return res;
				} else {
					res.assign(*this);
				}

				return res;
			}

			template<typename Input, typename Output>
			void customEval(const Input &input, Output &output) {
				if constexpr ((bool)(Flags & internal::flags::HasCustomEval)) {
					m_operation.customEval(input, output);
				} else {
					LR_ASSERT(false, "Type does not support custom eval");
				}
			}

			LR_FORCE_INLINE Packet packet(i64 index) const {
				if constexpr ((bool)(Flags & internal::flags::RequireInput)) {
					return m_operation.packetOpInput(m_value, index);
				} else {
					return m_operation.packetOp(m_value.packet(index));
				}
			}

			LR_FORCE_INLINE Scalar scalar(i64 index) const {
				if constexpr ((bool)(Flags & internal::flags::RequireInput)) {
					return m_operation.scalarOpInput(m_value, index);
				} else {
					return m_operation.scalarOp(m_value.scalar(index));
				}
			}

			template<typename T>
			LR_NODISCARD("")
			std::string genKernel(std::vector<T> &vec, i64 &index) const {
				std::string kernel = m_value.genKernel(vec, index);
				std::string op	   = m_operation.genKernel();
				return fmt::format("({}({}))", op, kernel);
			}

			LR_NODISCARD("")
			std::string str(std::string format = "", const std::string &delim = " ",
							i64 stripWidth = -1, i64 beforePoint = -1, i64 afterPoint = -1,
							i64 depth = 0) const {
				return eval().str(format, delim, stripWidth, beforePoint, afterPoint, depth);
			}

		protected:
			const ValType &m_value;
			Operation m_operation {};
		};
	} // namespace unop
} // namespace librapid

// Provide {fmt} printing capabilities
#ifdef FMT_API
template<typename Unop, typename TYPE>
struct fmt::formatter<librapid::unop::CWiseUnop<Unop, TYPE>> {
	std::string formatStr = "{}";

	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) {
		formatStr = "{:";
		auto it	  = ctx.begin();
		for (; it != ctx.end(); ++it) {
			if (*it == '}') break;
			formatStr += *it;
		}
		formatStr += "}";
		return it;
	}

	template<typename FormatContext>
	auto format(const librapid::unop::CWiseUnop<Unop, TYPE> &arr, FormatContext &ctx) {
		try {
			return fmt::format_to(ctx.out(), arr.str(formatStr));
		} catch (std::exception &e) { return fmt::format_to(ctx.out(), e.what()); }
	}
};
#endif // FMT_API