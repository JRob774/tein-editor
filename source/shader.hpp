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

TEINAPI Shader load_shader_from_source (std::string               source);
TEINAPI Shader load_shader_from_file   (std::string            file_name);
TEINAPI Shader load_shader_from_data   (const std::vector<u8>& file_data);
TEINAPI void   free_shader             (Shader                   program);

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
