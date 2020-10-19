// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// opengl
#include "opengl.hpp"

// project
#include "boids_renderer.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_mesh.hpp"


using namespace glm;
using namespace cgra;

BoidsRenderer::BoidsRenderer()
{
	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();
	m_shader = shader;
}

void BoidsRenderer::draw(const glm::mat4& view, const glm::mat4 proj)
{
	mat4 modelview = view;

	glUseProgram(m_shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr(m_color));

	if (!meshes.empty()) {
		for each (gl_mesh mesh in meshes)
		{
			mesh.draw();
		}
	}
}
