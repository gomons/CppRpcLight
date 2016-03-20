#pragma once

#include <cstdint>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/string.hpp>

namespace cpp_rpc_light
{
	class Header
	{
	public:
		uint32_t id;
		std::string func_name;

	private:
		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & id;
			ar & func_name;
		}
	};

	template<typename ReturnType, typename BodyType>
	class Pack
	{
	public:
		typedef ReturnType ReturnType;
		typedef BodyType BodyType;

		Header header;
		BodyType body;

	private:
		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & header;
			ar & body;
		}
	};

}; // namespace cpp_rpc_light