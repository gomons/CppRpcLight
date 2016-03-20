#pragma once

#include <memory>
#include <sstream>
#include "Pack.h"

namespace cpp_rpc_light
{
	template<typename T>
	inline std::shared_ptr<std::string> SerializeToBuffer(const T &pack)
	{
		std::stringstream buffer_stream;
		boost::archive::binary_oarchive out_archive(buffer_stream);
		out_archive << pack;
		buffer_stream.seekg(0, std::ios::end);
		uint32_t buffer_stream_size = static_cast<uint32_t>(buffer_stream.tellg());
		buffer_stream.seekg(0, std::ios::beg);

		auto buffer_ptr = std::make_shared<std::string>();
		buffer_ptr->resize(sizeof(uint32_t));
		*reinterpret_cast<uint32_t*>(&*buffer_ptr->begin()) = buffer_stream_size;
		*buffer_ptr += buffer_stream.str();

		return buffer_ptr;
	}

} // namespace cpp_rpc_light