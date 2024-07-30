#include "Shader.h"

Shader::Shader(std::string_view name)
{
	static std::filesystem::path shaderDir; // will store the current directory
	if (shaderDir.empty())
	{
		wchar_t moduleFileName[MAX_PATH]; 
		GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH); // gets the name of the current window module's file

		shaderDir = moduleFileName;
		shaderDir.remove_filename(); // remove the app.exe at the end
	}

	std::ifstream shaderIn(shaderDir / name, std::ios::binary);
	if (shaderIn.is_open())
	{
		shaderIn.seekg(0, std::ios::end); // seek to eof
		m_size = shaderIn.tellg(); // this gives the filesize by bytes 
		shaderIn.seekg(0, std::ios::beg); // seek to bof, to reset the stream
		m_data = malloc(m_size); // allocate the memory for the binary, get a pointer to that memory back

		if (m_data)
		{
			shaderIn.read((char*)m_data, m_size); // read the data to the allocated memory, byte by byte (hence the (char*))
		}
	}
}

Shader::~Shader()
{
	if (m_data)
	{
		free(m_data);
	}
}
