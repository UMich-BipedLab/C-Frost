#ifndef IPOPTPROBLEM_H
#define IPOPTPROBLEM_H

#include "rapidjson/document.h"

namespace frost {
	void IpoptConstraint(double *c, const double *x, const rapidjson::Document &document);
	void IpoptObjective(double &o, const double *x, const rapidjson::Document &document);
	void IpoptJacobian(double *Jval, const double *x, const rapidjson::Document &document);
	void IpoptGradient(double *Jgrad, const double *x, const rapidjson::Document &document);
}

#endif
