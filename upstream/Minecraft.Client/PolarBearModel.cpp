#include "stdafx.h"
#include "PolarBearModel.h"
#include "ModelPart.h"

PolarBearModel::PolarBearModel() : QuadrupedModel(10, 0)
{
	texWidth = 128;
	texHeight = 64;

	float g = 0;

	head = new ModelPart(this, 0, 0);
	head->addBox(-3.5f, -3, -3, 7, 7, 7, g);
	head->setPos(0, 10, -16);
	head->texOffs(0, 44)->addBox(-2.5f, 1, -6, 5, 3, 3, g);
	head->texOffs(26, 0)->addBox(-4.5f, -4, -1, 2, 2, 1, g);
	head->texOffs(26, 0)->addBox(2.5f, -4, -1, 2, 2, 1, g);

	body = new ModelPart(this, 0, 19);
	body->addBox(-5, -13, -7, 14, 14, 11, g);
	body->texOffs(39, 0)->addBox(-4, -25, -7, 12, 12, 10, g);
	body->setPos(-2, 9, 12);

	leg0 = new ModelPart(this, 50, 22);
	leg0->addBox(-2, 0, -2, 4, 10, 8, g);
	leg0->setPos(-4.5f, 14, 6);

	leg1 = new ModelPart(this, 50, 22);
	leg1->addBox(-2, 0, -2, 4, 10, 8, g);
	leg1->setPos(4.5f, 14, 6);

	leg2 = new ModelPart(this, 50, 40);
	leg2->addBox(-2, 0, -2, 4, 10, 6, g);
	leg2->setPos(-3.5f, 14, -8);

	leg3 = new ModelPart(this, 50, 40);
	leg3->addBox(-2, 0, -2, 4, 10, 6, g);
	leg3->setPos(3.5f, 14, -8);

	head->compile(1.0f / 16.0f);
	body->compile(1.0f / 16.0f);
	leg0->compile(1.0f / 16.0f);
	leg1->compile(1.0f / 16.0f);
	leg2->compile(1.0f / 16.0f);
	leg3->compile(1.0f / 16.0f);
}

void PolarBearModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	if (young)
	{
		glPushMatrix();
		glScalef(0.6666667f, 0.6666667f, 0.6666667f);
		glTranslatef(0, scale * 16.0f, scale * 4.0f);
		head->render(scale, usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(0.5f, 0.5f, 0.5f);
		glTranslatef(0, scale * 24.0f, 0);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
		glPopMatrix();
	}
	else
	{
		head->render(scale, usecompiled);
		body->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		leg2->render(scale, usecompiled);
		leg3->render(scale, usecompiled);
	}
}
