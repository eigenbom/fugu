/**
 * fg offline mode
 * @author ben
 */

#include "fg/fg.h"
#include "fg/functions.h"
#include "fg/glrenderer.h"
#include "fg/mesh.h"
#include "fg/meshimpl.h"

#include "vcgext/export_obj_point3d.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

int main(int argc, char *argv[])
{
	if (argc!=5){
		std::cout << "Usage: " << argv[0] << " <script> <prefix> <dt> <numframes>\n"
				<< "An example <script> is \"tests/basic5\" (note no .lua suffix needed\n"
				<< "prefix is the prefix of the .obj files\n"
				<< "dt is the step-size\n"
				<< "numframes is the number of frames\n";
		return 1;
	}
	else {
		std::cout << "fgo: fugu offline (c) 2011\n";
	}

	// Create a new universe
	fg::Universe u = fg::Universe();
	u.addScriptDirectory("../scripts/?.lua");
	u.loadScript(argv[1]);

	std::string prefix(argv[2]);

	double dt = 0.01;
	std::istringstream ssDT(argv[3]);
	ssDT >> dt;

	int numFrames = 1;
	std::istringstream ssNF(argv[4]);
	ssNF >> numFrames;

	double time = 0;
	for(int i=0;i<numFrames;i++){
		std::cout << "frame " << i << std::endl;

		// Update the universe
		u.update(dt);

		BOOST_FOREACH(boost::shared_ptr<fg::MeshNode> m, u.meshNodes()){
			m->mesh()->sync(); // make sure normals are okay
			// fg::GLRenderer::renderMeshNode(m,fg::GLRenderer::RenderMeshMode(DRAW_MODE));
			std::ostringstream oss;
			oss << prefix << "fgv" << std::setfill('0') << std::setw(4) << i << ".obj";
			// std::cout << "Saving as: \"" << oss.str().c_str() << "\"\n";
			vcg::tri::io::ExporterOBJ_Point3d<fg::MeshImpl>::Save(
				*m->mesh()->_impl(),
				oss.str().c_str(),
				vcg::tri::io::Mask::IOM_VERTNORMAL
			);
		}
	}

	return EXIT_SUCCESS;
}