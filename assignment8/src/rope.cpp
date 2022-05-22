#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

//        Comment-in this part when you implement the constructor
//        for (auto &i : pinned_nodes) {
//            masses[i]->pinned = true;
//        }
        Vector2D step = (end - start) / (num_nodes - 1);
        Vector2D current_pos = start;
        for(int i = 0; i < num_nodes; ++i) {
            masses.push_back(new Mass(current_pos, node_mass, false));
            current_pos += step;
        }
        for(int i = 0; i < num_nodes - 1; ++i) {
            springs.push_back(new Spring(masses[i], masses[i + 1], k));
        }
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        Vector2D springDir;
        double springLength;
        Vector2D springForce;
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            springDir = s->m2->position - s->m1->position;
            springLength = springDir.norm();
            springDir = springDir.unit();
            springForce = (springLength - s->rest_length) * s->k * springDir;
            s->m1->forces += springForce;
            s->m2->forces += -springForce;
        }

        for (auto &m : masses) {
            m->forces += gravity;
        }
        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                Vector2D acceleration = m->forces / m->mass;
                // m->position += m->velocity * delta_t;
                m->velocity += (1.0f - 0.00005f) * acceleration * delta_t;
                m->position += m->velocity * delta_t;
                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        Vector2D springDir;
        double springLength;
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            springDir = s->m2->position - s->m1->position;
            springLength = springDir.norm();
            springDir = springDir.unit();
            Vector2D posOffset = (springLength - s->rest_length) * springDir;
            if(s->m1->pinned && !s->m2->pinned) {
                s->m2->position -= posOffset;
            } else if(!s->m1->pinned && s->m2->pinned) {
                s->m1->position += posOffset;
            } else if(!s->m1->pinned && !s->m2->pinned) {
                s->m1->position += posOffset * 0.5f;
                s->m2->position -= posOffset * 0.5f;
            }
        }

        for (auto &m : masses) {
            m->forces += gravity;
        }
        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                Vector2D acceleration = m->forces / m->mass;
                float damping_factor = 0.00005f;
                m->position = m->position + (1.0f - damping_factor) * (m->position - m->last_position) + m->forces * delta_t * delta_t;
                m->last_position = temp_position;
                // TODO (Part 3.1): Set the new position of the rope mass
                
                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}
