/*******************************************************************************
 * Facilities for loading and creating OpenGL vert/frag shader programs.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

typedef GLuint Shader;

STDDEF Shader load_shader_from_source (std::string               source);
FILDEF Shader load_shader_from_file   (std::string            file_name);
FILDEF Shader load_shader_from_data   (const std::vector<u8>& file_data);
FILDEF void   free_shader             (Shader                   program);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
