#pragma once

typedef GLuint Shader;

TEINAPI Shader LoadShaderFromSource (std::string              source);
TEINAPI Shader LoadShaderFromData   (const std::vector<U8>& fileData);
TEINAPI Shader LoadShaderFromFile   (std::string            fileName);
TEINAPI void   FreeShader           (Shader                  program);
