/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "glviewwidget.h"

#include "mainwindow.h"
#include <QtGui/QMouseEvent>
#include <QGLContext>
#include "optionsmenu.h"
#include "graphwidget.h"
#include "trackmesh.h"
#include "myshader.h"
#include "mytexture.h"
#include "myframebuffer.h"

#include "osx/common.h"
#include <string>


#define FFAR (4000.f)
#define FNEAR (0.1f)
#define FOV (90)

#define DFAR (4000.)
#define DNEAR (0.1)

#define SHADOWMAP_SIZE (2048)
#define SHADOWMAP_RANGE (280.f)

#define SUPERSAMPLES 1
#define UNDERSAMPLES 1

#define OCULUS_SCALE (0.68f)

#ifdef USE_OVR
using namespace OVR;
#endif

extern MainWindow* gloParent;

glViewWidget::glViewWidget(QWidget *parent) : QGLWidget(parent) {
	QGLFormat format = QGLFormat::defaultFormat();
	format.setSampleBuffers(true);
	format.setOverlay(false);
	setFormat(format);
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent,true);

	simpleShadowFb = NULL;
	shadowVolumeFb = NULL;
	normalMapFb = NULL;
	occlusionFb = NULL;
	preDistortionFb = NULL;
	floorOpacity = 0.8;
	initialized = 0;
}

glViewWidget::~glViewWidget()
{
	if(!legacyMode)
	{
		delete skyShader;
		delete floorShader;
		delete trackShader;
		delete simpleSMShader;
		delete shadowVolumeShader;

		delete rasterTexture;
		delete metalTexture;
		delete floorTexture;
		delete skyTexture;

		delete simpleShadowFb;
		delete shadowVolumeFb;
	}
#ifdef USE_OVR
	if(riftMode)
	{
		OVR::System::Destroy();
		delete sensFusion;
		delete sensor;
	}
#endif
}

void glViewWidget::drawFloor()
{
	floorShader->bind();

	floorShader->useUniform("projectionMatrix", &ProjectionMatrix);
	floorShader->useUniform("modelMatrix", &ModelMatrix);
	floorShader->useUniform("eyePos", &cameraPos);
	floorShader->useUniform("rasterTex", rasterTexture->getId());
	floorShader->useUniform("floorTex", floorTexture->getId());

	if(shadowMode == 0) floorShader->useUniform("shadowTex", simpleShadowFb->getTexture());
	else if(shadowMode > 0)  floorShader->useUniform("shadowTex", shadowVolumeFb->getTexture());

	floorShader->useUniform("border", (GLuint)drawBorder);
	floorShader->useUniform("grid", (GLuint) gloParent->mOptions->drawGrid);
	floorShader->useUniform("opacity", floorOpacity);
	glBindVertexArray(floorMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 31);
}

void glViewWidget::drawSky()
{
	skyShader->bind();
	glm::mat4 inverse = glm::inverse(ProjectionModelMatrix);

	glm::vec4 topLeft = inverse * glm::vec4(-1, 1, 1, 1);
	topLeft /= topLeft.w;
	topLeft = -glm::vec4(glm::normalize(glm::vec3(topLeft)-cameraPos) ,0);

	glm::vec4 topRight = inverse * glm::vec4(1, 1, 1, 1);
	topRight /= topRight.w;
	topRight = -glm::vec4(glm::normalize(glm::vec3(topRight)-cameraPos) ,0);

	glm::vec4 bottomLeft = inverse * glm::vec4(-1, -1, 1, 1);
	bottomLeft /= bottomLeft.w;
	bottomLeft = -glm::vec4(glm::normalize(glm::vec3(bottomLeft)-cameraPos) ,0);

	glm::vec4 bottomRight = inverse * glm::vec4(1, -1, 1, 1);
	bottomRight /= bottomRight.w;
	bottomRight = -glm::vec4(glm::normalize(glm::vec3(bottomRight)-cameraPos) ,0);

	skyShader->useUniform("TL", topLeft.x, topLeft.y, topLeft.z);
	skyShader->useUniform("TR", topRight.x, topRight.y, topRight.z);
	skyShader->useUniform("BL", bottomLeft.x, bottomLeft.y, bottomLeft.z);
	skyShader->useUniform("BR", bottomRight.x, bottomRight.y, bottomRight.z);
	skyShader->useUniform("skyTex", skyTexture->getId());
	glBindVertexArray(skyMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void glViewWidget::drawTrack(trackHandler *_track, bool toNormalMap)
{
	myShader* shader;
	if(toNormalMap)
	{
		shader = normalMapShader;
		glDisable(GL_BLEND);
		normalMapFb->bind();
	}
	else
	{
		shader = trackShader;
	}

    if(!_track->mMesh->isInit) {
        _track->mMesh->init();
    }

	trackMesh* mesh = _track->mMesh;
	track* myTrack = _track->trackData;

	glm::mat4 anchorBase = glm::translate(myTrack->startPos) * glm::rotate(TO_RAD(myTrack->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));

	shader->bind();
	shader->useUniform("projectionMatrix", &ProjectionMatrix);
	shader->useUniform("modelMatrix", &ModelMatrix);
	shader->useUniform("anchorBase", &anchorBase);
	shader->useUniform("eyePos", &cameraPos);
	shader->useUniform("colorMode", (GLuint)curTrackShader);

	shader->useUniform("metalTex", metalTexture->getId());
	shader->useUniform("skyTex", skyTexture->getId());
	shader->useUniform("occlusionTex", occlusionFb->getTexture());

	if(shadowMode == 0) shader->useUniform("shadowTex", simpleShadowFb->getTexture());
	else if(shadowMode > 0)  shader->useUniform("shadowTex", shadowVolumeFb->getTexture());

	QColor* tempColor = _track->trackColors;
	shader->useUniform("defaultColor", tempColor[0].red()/255.f, tempColor[0].green()/255.f, tempColor[0].blue()/255.f);
	shader->useUniform("sectionColor", tempColor[1].red()/255.f, tempColor[1].green()/255.f, tempColor[1].blue()/255.f);
	shader->useUniform("transitionColor", tempColor[2].red()/255.f, tempColor[2].green()/255.f, tempColor[2].blue()/255.f);

	shader->useUniform("lightDir", &lightDir);

	if(myTrack->drawHeartline != 2 && myTrack->lSections.size()!=0)
	{
		glBindVertexArray(mesh->TrackObject[0]);
		for(int i = 0; i < mesh->pipeBorders.size()-1; ++i)
		{
			//glDrawArrays(GL_TRIANGLE_STRIP, i*mesh->trackVertexSize, mesh->trackVertexSize);
			if(mesh->isWireframe)
			{
				glDrawElements(GL_LINE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
			}
			else
			{
				glDrawElements(GL_TRIANGLE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
			}
			//glDrawArrays(GL_POINTS, i*mesh->trackVertexSize, mesh->trackVertexSize);
		}
		glBindVertexArray(mesh->TrackObject[3]);
		if(mesh->isWireframe)
		{
			glDrawArrays(GL_LINES, 0, mesh->crossties.size());
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, mesh->crossties.size());
		}
		//glDrawArrays(GL_POINTS, 0, mesh->crossties.size());


		glBindVertexArray(mesh->TrackObject[4]);
		if(mesh->isWireframe)
		{
			shader->useUniform("defaultColor", 0.6f, 0.2f, 0.2f);
			glDrawArrays(GL_LINES, 0, mesh->rendersupports.size());
		}
		else
		{
			shader->useUniform("defaultColor", 0.6f, 0.6f, 0.6f);
			for(int i = 0; i < mesh->supportsSize; ++i)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i*61, 61);
			}
			//glDrawArrays(GL_POINTS, i*61, 61);
		}
	}

	shader->useUniform("defaultColor", 0.9f, 0.9f, 0.4f);
	if(myTrack->drawHeartline != 1)
	{
		glBindVertexArray(mesh->HeartObject[0]);
		glDrawArrays(GL_LINE_STRIP, 0, mesh->heartline.size());
	}

	if(toNormalMap)
	{
		normalMapFb->unbind();
		glEnable(GL_BLEND);
	}
}

void glViewWidget::drawSimpleSM(trackHandler *_track)
{
	trackMesh* mesh = _track->mMesh;
	track* myTrack = _track->trackData;

	glm::mat4 anchorBase = glm::translate(myTrack->startPos) * glm::rotate(TO_RAD(myTrack->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));

	if(shadowMode == 0) simpleShadowFb->bind();
	else
	{
		glDisable(GL_DEPTH_TEST);
		shadowVolumeFb->bind();
	}

	simpleSMShader->bind();
	simpleSMShader->useUniform("projectionMatrix", &ProjectionMatrix);
	simpleSMShader->useUniform("modelMatrix", &ModelMatrix);
	simpleSMShader->useUniform("anchorBase", &anchorBase);

	simpleSMShader->useUniform("lightDir", &lightDir);

	if(myTrack->drawHeartline != 2)
	{
		glBindVertexArray(mesh->TrackObject[0]);
		for(int i = 0; i < mesh->pipeBorders.size()-1; ++i)
		{
			if(mesh->isWireframe)
			{
				glDrawElements(GL_LINE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
			}
			else
			{
				glDrawElements(GL_TRIANGLE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
			}
		}
		glBindVertexArray(mesh->TrackObject[3]);
		if(mesh->isWireframe)
		{
			glDrawArrays(GL_LINES, 0, mesh->crossties.size());
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, mesh->crossties.size());
		}


		glBindVertexArray(mesh->TrackObject[4]);
		if(mesh->isWireframe)
		{
			glDrawArrays(GL_LINES, 0, mesh->rendersupports.size());
		}
		else
		{
			for(int i = 0; i < mesh->supportsSize; ++i)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i*61, 61);
			}
		}
	}

	if(myTrack->drawHeartline == 2)
	{
		glBindVertexArray(mesh->HeartObject[0]);
		glDrawArrays(GL_LINE_STRIP, 0, mesh->heartline.size());
	}

	if(shadowMode == 0) simpleShadowFb->unbind();
	else
	{
		glEnable(GL_DEPTH_TEST);
		shadowVolumeFb->unbind();
	}
	glClearColor(0, 0, 0, 1);
}

void glViewWidget::drawShadowVolumes()
{
	QList<trackHandler*> trackList = gloParent->getTrackList();
	shadowVolumeFb->bind();


	shadowVolumeShader->bind();
	glm::mat4 anchorBase = glm::translate(glm::vec3(0, 0, 0));

	shadowVolumeShader->useUniform("projectionMatrix", &ProjectionMatrix);
	shadowVolumeShader->useUniform("modelMatrix", &ModelMatrix);
	shadowVolumeShader->useUniform("anchorBase", &anchorBase);
	shadowVolumeShader->useUniform("uFill", 0.f);

	/* RENDER MESHES */

	glBindVertexArray(floorMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 31);
	for(int i = 0; i < trackList.size(); ++i)
	{
		if(trackList[i]->trackData->drawTrack)
		{
            if(!trackList[i]->mMesh->isInit) {
                trackList[i]->mMesh->init();
            }

			trackMesh* mesh = trackList[i]->mMesh;
			track* myTrack = trackList[i]->trackData;
			anchorBase = glm::translate(myTrack->startPos) * glm::rotate(TO_RAD(myTrack->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));
			shadowVolumeShader->useUniform("anchorBase", &anchorBase);
			if(myTrack->drawHeartline != 2 && myTrack->lSections.size()!=0)
			{
				glBindVertexArray(mesh->TrackObject[0]);
				for(int i = 0; i < mesh->pipeBorders.size()-1; ++i)
				{
					if(mesh->isWireframe)
					{
						glDrawElements(GL_LINE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
					}
					else
					{
						glDrawElements(GL_TRIANGLE_STRIP, mesh->pipeBorders[i+1]-mesh->pipeBorders[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint)*mesh->pipeBorders[i]));
					}
				}
				glBindVertexArray(mesh->TrackObject[3]);
				if(mesh->isWireframe)
				{
					glDrawArrays(GL_LINES, 0, mesh->crossties.size());
				}
				else
				{
					glDrawArrays(GL_TRIANGLES, 0, mesh->crossties.size());
				}


				glBindVertexArray(mesh->TrackObject[4]);
				if(mesh->isWireframe)
				{
					glDrawArrays(GL_LINES, 0, mesh->rendersupports.size());
				}
				else
				{
					for(int i = 0; i < mesh->supportsSize; ++i)
					{
						glDrawArrays(GL_TRIANGLE_STRIP, i*61, 61);
					}
				}
			}
		}
	}

	/* RENDER SHADOW VOLUMES NOW */

	glColorMask(false, false, false, false);
	glDepthMask(false);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x00, 0xff);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	for(int i = 0; i < trackList.size(); ++i)
	{
		if(trackList[i]->trackData->drawTrack && trackList[i]->trackData->drawHeartline != 2 && trackList[i]->trackData->lSections.size()!=0)
		{
			trackMesh* mesh = trackList[i]->mMesh;
			track* myTrack = trackList[i]->trackData;
			anchorBase = glm::translate(myTrack->startPos) * glm::rotate(TO_RAD(myTrack->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));
			shadowVolumeShader->useUniform("anchorBase", &anchorBase);
			glBindVertexArray(mesh->HeartObject[1]);
			glDrawElements(GL_TRIANGLES, mesh->shadowIndices.size(), GL_UNSIGNED_INT, (GLvoid*)0);
			glBindVertexArray(mesh->HeartObject[3]);
			glDrawArrays(GL_TRIANGLES, 0, mesh->supportshadows.size());
			glBindVertexArray(mesh->HeartObject[4]);
			glDrawArrays(GL_TRIANGLES, 0, mesh->crosstieshadows.size());
		}
	}

	glColorMask(true, true, true, true);
	glDepthMask(true);
	glEnable(GL_STENCIL_TEST);

	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0x0, 0xFF);

	/* RENDER MASK */

	glDisable(GL_DEPTH_TEST);

	anchorBase = glm::translate(glm::vec3(0, 0, 0));

	shadowVolumeShader->useUniform("uFill", 1.f);
	shadowVolumeShader->useUniform("projectionMatrix", &anchorBase);
	shadowVolumeShader->useUniform("modelMatrix", &anchorBase);
	shadowVolumeShader->useUniform("anchorBase", &anchorBase);
	glBindVertexArray(skyMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	shadowVolumeFb->unbind();
}

void glViewWidget::drawOcclusion()
{
	QList<trackHandler*> trackList = gloParent->getTrackList();
	for(int i = 0; i < trackList.size(); ++i)
	{
		if(trackList[i]->trackData->drawTrack)
        {
            if(!trackList[i]->mMesh->isInit) {
                trackList[i]->mMesh->init();
            }

            if(trackList[i]->trackData->hasChanged)
			{
				trackList[i]->mMesh->recolorTrack();
				trackList[i]->trackData->hasChanged = false;
			}
			if(!trackList[i]->mMesh->isWireframe) drawTrack(trackList[i], true);
		}
	}


	occlusionFb->bind();

	occlusionShader->bind();

	glm::mat4 inverse = glm::inverse(ProjectionModelMatrix);

	glm::vec4 topLeft = inverse * glm::vec4(-1, 1, 1, 1);
	topLeft /= topLeft.w;
	topLeft = -glm::vec4(glm::normalize(glm::vec3(topLeft)-cameraPos) ,0);

	glm::vec4 topRight = inverse * glm::vec4(1, 1, 1, 1);
	topRight /= topRight.w;
	topRight = -glm::vec4(glm::normalize(glm::vec3(topRight)-cameraPos) ,0);

	glm::vec4 bottomLeft = inverse * glm::vec4(-1, -1, 1, 1);
	bottomLeft /= bottomLeft.w;
	bottomLeft = -glm::vec4(glm::normalize(glm::vec3(bottomLeft)-cameraPos) ,0);

	glm::vec4 bottomRight = inverse * glm::vec4(1, -1, 1, 1);
	bottomRight /= bottomRight.w;
	bottomRight = -glm::vec4(glm::normalize(glm::vec3(bottomRight)-cameraPos) ,0);

	occlusionShader->useUniform("TL", topLeft.x, topLeft.y, topLeft.z);
	occlusionShader->useUniform("TR", topRight.x, topRight.y, topRight.z);
	occlusionShader->useUniform("BL", bottomLeft.x, bottomLeft.y, bottomLeft.z);
	occlusionShader->useUniform("BR", bottomRight.x, bottomRight.y, bottomRight.z);
	occlusionShader->useUniform("eyePos", &cameraPos);
	occlusionShader->useUniform("tex", normalMapFb->getTexture());
	occlusionShader->useUniform("width", 1.f/viewPortWidth);
	occlusionShader->useUniform("height", 1.f/viewPortHeight);
	glBindVertexArray(skyMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	occlusionFb->unbind();
}

void glViewWidget::drawDebug()
{
	glDisable(GL_DEPTH_TEST);
	debugShader->bind();
	debugShader->useUniform("tex", preDistortionFb->getTexture());
	glBindVertexArray(skyMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
}

void glViewWidget::drawOculus()
{
	glDisable(GL_DEPTH_TEST);
	oculusShader->bind();
	oculusShader->useUniform("tex", preDistortionFb->getTexture());
	oculusShader->useUniform("hmdWarp", &HmdWarp);
	float lensCenter = 0.5f-1.f*lensSep/hScreenSize;
	oculusShader->useUniform("lensCenter", lensCenter);
	oculusShader->useUniform("scale", OCULUS_SCALE);
	glBindVertexArray(skyMesh.object);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
}

void glViewWidget::paintGL()
{
	if(initialized != 2) return;
	if(gloParent->mGraphWidget) gloParent->mGraphWidget->setBezPoints();
	if(!paintMode) return;

	fov = gloParent->mOptions->fov;
	shadowMode = gloParent->mOptions->shadowQuality;
	setBackgroundColor(gloParent->mOptions->backgroundColor);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	renderTime = frameTimer.nsecsElapsed()/1000000000.f;
	frameTimer.start();

	/*	static float avgTime = 0.f;
	static int frames = 0;
	frames++;
	avgTime+=renderTime;
	if(frames%60 == 0) {
		qDebug("%4.1f fps", 60.f/avgTime);
		avgTime = 0.f;
	}*/

	if(!legacyMode)
	{
		moveCamera();

		if(!riftMode/* || !gloParent->isFullScreen()*/)
		{
			buildMatrices(0);
			if(shadowMode == 0)   simpleShadowFb->clear();
			else if(shadowMode > 0) shadowVolumeFb->clear();
			occlusionFb->clear();
			normalMapFb->clear();

			//if(moveMode) updateLoD();

			if(shadowMode > 0)
			{
				drawShadowVolumes();
			}

			if(shadowMode > 1) drawOcclusion();

			QList<trackHandler*> trackList = gloParent->getTrackList();
			for(int i = 0; i < trackList.size(); ++i)
			{
				if(trackList[i]->trackData->drawTrack)
				{
					if(trackList[i]->trackData->hasChanged)
					{
						trackList[i]->mMesh->recolorTrack();
						trackList[i]->trackData->hasChanged = false;
					}
					if(shadowMode == 0 || trackList[i]->mMesh->isWireframe || trackList[i]->trackData->drawHeartline == 2)
					{
						drawSimpleSM(trackList[i]);
					}
				}
			}

			for(int i = 0; i < trackList.size(); ++i)
			{
				if(trackList[i]->trackData->drawTrack)
				{
					drawTrack(trackList[i]);
				}
			}
			drawFloor();
			drawSky();
		}
		else // rift mode
		{
			preDistortionFb->clear();
			for(int p = 0; p < 2; ++p)
			{
				buildMatrices((p-0.5f)*fIPD);
				glViewport(0, 0, 2*viewPortWidth/2, 2*viewPortHeight);

				if(shadowMode == 0)   simpleShadowFb->clear();
				else if(shadowMode > 0) shadowVolumeFb->clear();
				occlusionFb->clear();
				normalMapFb->clear();

				//if(moveMode) updateLoD();

				if(shadowMode > 0)
				{
					drawShadowVolumes();
				}

				if(shadowMode > 1) drawOcclusion();

				QList<trackHandler*> trackList = gloParent->getTrackList();
				for(int i = 0; i < trackList.size(); ++i)
				{
					if(trackList[i]->trackData->drawTrack)
					{
						if(trackList[i]->trackData->hasChanged)
						{
							trackList[i]->mMesh->recolorTrack();
							trackList[i]->trackData->hasChanged = false;
						}
						if(shadowMode == 0 || trackList[i]->mMesh->isWireframe || trackList[i]->trackData->drawHeartline == 2)
						{
							drawSimpleSM(trackList[i]);
						}
					}
				}

				glViewport(p*2*viewPortWidth/2, 0, 2*viewPortWidth/2, 2*viewPortHeight);

				preDistortionFb->bind();
				for(int i = 0; i < trackList.size(); ++i)
				{
					if(trackList[i]->trackData->drawTrack)
					{
						drawTrack(trackList[i]);
					}
				}
				drawFloor();
				buildMatrices(0);
				drawSky();
				preDistortionFb->unbind();
			}
			glViewport(0, 0, viewPortWidth, viewPortHeight);
			drawOculus();
		}
	}
	else
	{
		moveCamera();
		buildMatrices(0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if(!povMode)
		{
			gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraPos.x+freeFlyDir.x, cameraPos.y+freeFlyDir.y, cameraPos.z+freeFlyDir.z, 0.0, 1.0, 0.0);
		}
		else
		{
			glm::mat4 anchorBase = glm::translate(gloParent->curTrack()->startPos) * glm::rotate(TO_RAD(gloParent->curTrack()->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));
			glm::vec3 vNormal = glm::vec3(anchorBase * glm::vec4(-povNode->vNorm, 0.f));
			glm::vec3 vLookAt = glm::vec3(anchorBase * glm::vec4(povNode->vPos+povNode->vDirHeart(gloParent->curTrack()->fHeart), 1.f));
			glm::vec3 vPosition = glm::vec3(anchorBase * glm::vec4(povNode->vPos, 1.f));
			gluLookAt(vPosition.x, vPosition.y, vPosition.z, vLookAt.x, vLookAt.y, vLookAt.z, vNormal.x, vNormal.y, vNormal.z);
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov*0.5, this->width()/((double)this->height()), DNEAR, DFAR);

		legacyDrawFloor();
		QList<trackHandler*> trackList = gloParent->getTrackList();
		for(int i = 0; i < trackList.size(); ++i)
		{
			if(trackList[i]->trackData->drawTrack)
			{
				legacyDrawTrack(trackList[i]);
			}
		}
	}
}

QString glViewWidget::getGLVersionString()
{
	return QString((const char*)glGetString(GL_VERSION));
}

bool glViewWidget::loadGroundTexture(QString fileName)
{
	QImage img(fileName);
	if(img.isNull())
	{
		return false;
	}
	if(floorTexture)
	{
		floorTexture->changeTexture(img);
	}
	else
	{
		floorTexture = new myTexture(img, 2);
	}
	return true;
}

void glViewWidget::setBackgroundColor(QColor _background)
{
	//if(legacyMode)
	{
		clearColor = _background;
		glClearColor(_background.red()/255.f, _background.green()/255.f, _background.blue()/255.f, 1.f);
	}
	/*else
	{
		clearColor = _background;
		glClearColor(1, 1, 1, 1);
	}*/
}

void glViewWidget::initializeGL()
{
	if(initialized) return;
	initialized++;

	myTexture::initialize();
	drawBorder = 0;
	moveMode = false;
	povMode = false;
	povPos = 0;
	cameraJump = 0;
	povNode = NULL;
	paintMode = true;
	riftMode = false;

	freeFlyPos = glm::vec3(0.0f, 10.0f, 20.0f);
	freeFlyDir = glm::vec3(0.0f, 0.0f, -1.0f);
	freeFlySide = glm::vec3(1.0f, 0.0f, 0.0f);
	cameraDir = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraMov = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraBoost = 1.0f;
	curTrackShader = 0;
	fov = gloParent->mOptions->fov;
	shadowMode = gloParent->mOptions->shadowQuality;

	lightDir = glm::normalize(glm::vec3(0, -1, 0));

	floorTexture = NULL;
	rasterTexture = NULL;
	metalTexture = NULL;
	skyTexture = NULL;

	qDebug("found OpenGL Context:");
	qDebug("Vendor: %s", glGetString(GL_VENDOR));
	qDebug("Renderer: %s", glGetString(GL_RENDERER));
	qDebug("Version: %s", glGetString(GL_VERSION));
	qDebug("Samples: %d %d", this->format().sampleBuffers(), this->format().samples());

#ifndef Q_OS_MAC // on Win / Unix
	if(glewInit() != GLEW_OK)
	{
		qDebug("GLEW initialization failed, aborting!");
		exit(4);
	}
	else
	{
		qDebug("GLEW initialization successful!");
	}
#endif
#ifdef Q_OS_MAC
#endif

	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_POLYGON_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	setBackgroundColor(gloParent->mOptions->backgroundColor);

	gloParent->initProject();

	viewPortHeight = this->height();
	viewPortWidth = this->width();

#ifdef USE_OVR
	if(riftMode)
	{
		System::Init(Log::ConfigureDefaultLog(LogMask_All));
		Ptr<DeviceManager> pManager = *DeviceManager::Create();
		Ptr<HMDDevice> pHMD = *pManager->EnumerateDevices<HMDDevice>(true).CreateDevice();
		HMDInfo info;
		sensor = NULL;
		sensFusion = NULL;
		if(pHMD && pHMD->GetDeviceInfo(&info))
		{
			fEyeToScreen = info.EyeToScreenDistance;
			fIPD = info.InterpupillaryDistance;
			HmdWarp.x = info.DistortionK[0];
			HmdWarp.y = info.DistortionK[1];
			HmdWarp.z = info.DistortionK[2];
			HmdWarp.w = info.DistortionK[3];
			lensSep = info.LensSeparationDistance;
			hScreenSize = info.HScreenSize;
			vScreenSize = info.VScreenSize;
			sensor = pHMD->GetSensor();
		}
		else
		{
			HmdWarp.x = 1.f;
			HmdWarp.y = 0.22f;
			HmdWarp.z = 0.24f;
			HmdWarp.w = 0.f;
			lensSep = 0.0635f;
			hScreenSize = 0.14976f;
			vScreenSize = 0.0936f;
			fEyeToScreen = 0.041f;
			fIPD = 0.0655f;
		}
		if(sensor)
		{
			sensFusion = new SensorFusion();
			sensFusion->AttachToSensor(sensor);
			qDebug("sensor fusion ready");
		}
	}
#endif
	if(!legacyMode)
	{
		glEnable(GL_DEPTH_TEST);

		initFloorMesh();
		initShaders();
		initTextures();


		/*initFloor();
		initFloorShader();

		initTrackShader();

		initHeartShader();

		initDeferred();*/
	}
	initialized++;
}

void glViewWidget::resizeGL(int w, int h)
{
	viewPortWidth = w;
	viewPortHeight = h;
	if(riftMode/* && gloParent->isFullScreen()*/)
	{
		glViewport(0, 0, w/2, h);
		if(simpleShadowFb) simpleShadowFb->resize(w/2, h);
		if(shadowVolumeFb) shadowVolumeFb->resize(w/2, h);
		if(normalMapFb) normalMapFb->resize(w/2, h);
		if(occlusionFb) occlusionFb->resize(w/2, h);
	}
	else
	{
		glViewport(0, 0, w, h);
		if(simpleShadowFb) simpleShadowFb->resize(w, h);
		if(shadowVolumeFb) shadowVolumeFb->resize(w, h);
		if(normalMapFb) normalMapFb->resize(w, h);
		if(occlusionFb) occlusionFb->resize(w, h);
	}
	if(preDistortionFb) preDistortionFb->resize(2*w, 2*h);
}

void glViewWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton)
	{
		moveMode = !moveMode;
	}
	else
	{
		return;
	}
	if(moveMode)
	{
		setCursor(QCursor(Qt::BlankCursor));
		mousePos = this->cursor().pos();
		if(this->mapFromGlobal(mousePos).x() < 80)
		{
			mousePos.setX(this->mapToGlobal(QPoint(80, this->mapFromGlobal(mousePos).y())).x());
		}
		else if(this->mapFromGlobal(mousePos).x() > this->width()-80)
		{
			mousePos.setX(this->mapToGlobal(QPoint(this->width()-80, this->mapFromGlobal(mousePos).y())).x());
		}
		if(this->mapFromGlobal(mousePos).y() < 80)
		{
			mousePos.setY(this->mapToGlobal(QPoint(this->mapFromGlobal(mousePos).x(), 80)).y());
		}
		else if(this->mapFromGlobal(mousePos).y() > this->height()-80)
		{
			mousePos.setY(this->mapToGlobal(QPoint(this->mapFromGlobal(mousePos).x(), this->height()-80)).y());
		}
		this->cursor().setPos(mousePos);

		this->setFocus();
		this->grabKeyboard();
		hasChanged = true;
		//this->cursor().setPos(this->mapToGlobal(QPoint(this->width()/2.0, this->height()/2.0)));
	}
	else
	{
		setCursor(QCursor(Qt::ArrowCursor));
		this->cursor().setPos(mousePos);
		cameraMov = glm::vec4(0.f, 0.f, 0.f, 0.f);

		this->releaseKeyboard();
	}
}

void glViewWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void glViewWidget::keyPressEvent(QKeyEvent *event)
{
	if(!moveMode) return;

	switch(event->key()) {
	case Qt::Key_W:
		cameraMov.x = 1.f;
		break;
	case Qt::Key_A:
		cameraMov.y = 1.f;
		break;
	case Qt::Key_S:
		cameraMov.z = 1.f;
		break;
	case Qt::Key_D:
		cameraMov.w = 1.f;
		break;
	case Qt::Key_Shift:
		cameraBoost = 2.0f;
		break;
	case Qt::Key_Control:
		cameraBoost = 0.5f;
		break;
	case Qt::Key_Escape:
		break;
	case Qt::Key_Space:
		if(!gloParent->curTrack()) break;
		if(!gloParent->curTrack()->lSections.size())
			povMode = true;
		povMode = !povMode;
		if(!povMode) gloParent->mGraphWidget->drawGraph(10); // 10 = povPos
		break;
	default:
		event->ignore();
		break;
	}
	event->accept();
}

void glViewWidget::keyReleaseEvent(QKeyEvent *event)
{
	if(!moveMode) return;


	switch(event->key()) {
	case Qt::Key_W:
		cameraMov.x = 0.f;
		break;
	case Qt::Key_A:
		cameraMov.y = 0.f;
		break;
	case Qt::Key_S:
		cameraMov.z = 0.f;
		break;
	case Qt::Key_D:
		cameraMov.w = 0.f;
		break;
	case Qt::Key_Shift:
		cameraBoost = 1.0f;
		break;
	case Qt::Key_Control:
		cameraBoost = 1.0f;
		break;
	case Qt::Key_Escape:
		break;
	case Qt::Key_F:
		if(gloParent->isFullScreen()) {
			gloParent->showAll();
			gloParent->showNormal();
		} else {
			gloParent->hideAll();
			gloParent->showFullScreen();
			//this->setWindowState(Qt::WindowFullScreen);
			//this->setMouseTracking(true);
			//this->setFocus();
		}
		break;
	case Qt::Key_B:
		drawBorder = 1-drawBorder;
		break;
	case Qt::Key_Left:
		if(povMode)
		{
			gloParent->curTrack()->povPos.x -= cameraBoost*0.1;
		}
		break;
	case Qt::Key_Right:
		if(povMode)
		{
			gloParent->curTrack()->povPos.x += cameraBoost*0.1;
		}
		break;
	case Qt::Key_Down:
		if(povMode)
		{
			gloParent->curTrack()->povPos.y -= cameraBoost*0.1;
		}
		break;
	case Qt::Key_Up:
		if(povMode)
		{
			gloParent->curTrack()->povPos.y += cameraBoost*0.1;
		}
		break;
	default:
		event->ignore();
		break;
	}
	event->accept();
}

void glViewWidget::wheelEvent(QWheelEvent *event)
{
	if(!moveMode) return;
	cameraJump -= event->delta();
}


void glViewWidget::initFloorMesh()
{
	glGenVertexArrays(1, &floorMesh.object);
	glGenBuffers(1, &floorMesh.buffer);

	glBindVertexArray(floorMesh.object);

	glBindBuffer(GL_ARRAY_BUFFER, floorMesh.buffer);

	/*float floor[36] = {0.f, 0.f, 0.f, 0.f, 0.5f, 0.f,               // STILL TO DO
					 8000.f, 0.f, 8000.f, 0.f, 0.5f, 0.f,
					 8000.f, 0.f, -8000.f, 0.f, 0.5f, 0.f,
					 -8000.f, 0.f, -8000.f, 0.f, 0.5f, 0.f,
					 -8000.f, 0.f, 8000.f, 0.f, 0.5f, 0.f,
					 8000.f, 0.f, 8000.f, 0.f, 0.5f, 0.f};*/


	float a = 8000.f;
	float b = 200.f;

	float floor[31*6] = {-a, 0.f, +a, 0.f, 0.5f, 0.f,               // STILL TO DO
						 -a, 0.f, +b, 0.f, 0.5f, 0.f,
						 -b, 0.f, +a, 0.f, 0.5f, 0.f,
						 -b, 0.f, +b, 0.f, 0.5f, 0.f,
						 +b, 0.f, +a, 0.f, 0.5f, 0.f,
						 +b, 0.f, +b, 0.f, 0.5f, 0.f,
						 +a, 0.f, +a, 0.f, 0.5f, 0.f,
						 +a, 0.f, +b, 0.f, 0.5f, 0.f,
						 +a, 0.f, +b, 0.f, 0.5f, 0.f,
						 +b, 0.f, +b, 0.f, 0.5f, 0.f,

						 +a, 0.f, -b, 0.f, 0.5f, 0.f,
						 +b, 0.f, -b, 0.f, 0.5f, 0.f,
						 +a, 0.f, -a, 0.f, 0.5f, 0.f,
						 +b, 0.f, -a, 0.f, 0.5f, 0.f,
						 +b, 0.f, -b, 0.f, 0.5f, 0.f,

						 -b, 0.f, -a, 0.f, 0.5f, 0.f,
						 -b, 0.f, -b, 0.f, 0.5f, 0.f,
						 -a, 0.f, -a, 0.f, 0.5f, 0.f,
						 -a, 0.f, -b, 0.f, 0.5f, 0.f,
						 -a, 0.f, -b, 0.f, 0.5f, 0.f,
						 -b, 0.f, -b, 0.f, 0.5f, 0.f,

						 -a, 0.f, +b, 0.f, 0.5f, 0.f,
						 -b, 0.f, +b, 0.f, 0.5f, 0.f,
						 -b, 0.f, +b, 0.f, 0.5f, 0.f,
						 -b, 0.f, -b, 0.f, 0.5f, 0.f,
						 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f,
						 +b, 0.f, -b, 0.f, 0.5f, 0.f,
						 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f,
						 +b, 0.f, +b, 0.f, 0.5f, 0.f,
						 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f,
						 -b, 0.f, +b, 0.f, 0.5f, 0.f
						};

	glBufferData(GL_ARRAY_BUFFER, 31*6*sizeof(float), floor, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	glGenVertexArrays(1, &skyMesh.object);
	glGenBuffers(1, &skyMesh.buffer);

	glBindVertexArray(skyMesh.object);

	glBindBuffer(GL_ARRAY_BUFFER, skyMesh.buffer);

	float sky[12] = {1.f, 1.f, 0.9999f,
					 1.f, -1.f, 0.9999f,
					 -1.f, 1.f, 0.9999f,
					 -1.f, -1.f, 0.9999f};

	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), sky, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void glViewWidget::initTextures()
{
#define RASTER_SIZE (600)
	QImage* raster = new QImage(RASTER_SIZE, RASTER_SIZE, QImage::Format_ARGB32);

	for(int i = 0; i < RASTER_SIZE; ++i)
	{
		for(int j = 0; j < RASTER_SIZE; ++j)
		{
			//float col = std::min(fabs(i-RASTER_SIZE/2.f), fabs(j-RASTER_SIZE/2.f))/(RASTER_SIZE/2.f);
			double major = std::min(fabs(i-RASTER_SIZE/2.f), fabs(j-RASTER_SIZE/2.f))/(RASTER_SIZE/2.f);
			double minor = std::min(std::min(fabs(((i+(RASTER_SIZE/20))%(RASTER_SIZE/10)-(RASTER_SIZE/20.f))), fabs(((j+(RASTER_SIZE/20))%(RASTER_SIZE/10)-(RASTER_SIZE/20.f)))), 1.);
			major = 1-major;
			minor = 1-minor;
			for(int k = 0; k < 9; ++k)
			{
				major *= major;
				minor *= minor;
			}
			major = 1-major;
			minor = 1-0.5*minor;
			major *= 255;
			minor *= 255;
			double col = std::min(major, minor);
			raster->setPixel(i, j, 0xff000000 + ((int)col<<16) + ((int)col<<8) + (int)col);
		}
	}
	rasterTexture = new myTexture(*raster, 1);
	loadGroundTexture(":/background.png");
#ifdef Q_OS_LINUX
	skyTexture = new myTexture(":/sky/negx.jpg", ":/sky/negy.jpg", ":/sky/negz.jpg", ":/sky/posx.jpg", ":/sky/posy.jpg", ":/sky/posz.jpg");
#endif
#ifdef Q_OS_WIN32
	skyTexture = new myTexture(":/sky/negx.jpg", ":/sky/negy.jpg", ":/sky/negz.jpg", ":/sky/posx.jpg", ":/sky/posy.jpg", ":/sky/posz.jpg");
#endif
#ifdef Q_OS_MAC
	skyTexture = new myTexture(":/sky/negx.jpg", ":/sky/negy.jpg", ":/sky/negz.jpg", ":/sky/posx.jpg", ":/sky/posy.jpg", ":/sky/posz.jpg");
#endif
	metalTexture = new myTexture(":/metal.png", 2);
	delete raster;
}

void glViewWidget::initShaders()
{
#ifdef Q_OS_LINUX
	floorShader = new myShader(":/shaders/floor.vert", ":/shaders/floor.frag");
#endif
#ifdef Q_OS_WIN32
	floorShader = new myShader(":/shaders/floor.vert", ":/shaders/floor.frag");
#endif
#ifdef Q_OS_MAC
	floorShader = new myShader(":/shaders/floor.vert", ":/shaders/floor.frag");
#endif
	floorShader->useAttribute(0, "aPosition");
	floorShader->useAttribute(1, "aNormal");
	floorShader->linkProgram();

#ifdef Q_OS_LINUX
	skyShader = new myShader(":/shaders/sky.vert", ":/shaders/sky.frag");
#endif
#ifdef Q_OS_WIN32
	skyShader = new myShader(":/shaders/sky.vert", ":/shaders/sky.frag");
#endif
#ifdef Q_OS_MAC
	skyShader = new myShader(":/shaders/sky.vert", ":/shaders/sky.frag");
#endif
	skyShader->useAttribute(0, "aPosition");
	skyShader->linkProgram();

#ifdef Q_OS_LINUX
	trackShader = new myShader(":/shaders/track.vert", ":/shaders/track.frag");
#endif
#ifdef Q_OS_WIN32
	trackShader = new myShader(":/shaders/track.vert", ":/shaders/track.frag");
#endif
#ifdef Q_OS_MAC
	trackShader = new myShader(":/shaders/track.vert", ":/shaders/track.frag");
#endif
	trackShader->useAttribute(0, "aPosition");
	trackShader->useAttribute(1, "aVel");
	trackShader->useAttribute(2, "aRoll");
	trackShader->useAttribute(3, "aNForce");
	trackShader->useAttribute(4, "aLForce");
	trackShader->useAttribute(5, "aFlex");
	trackShader->useAttribute(6, "aselected");
	trackShader->useAttribute(7, "aNormal");
	trackShader->useAttribute(8, "aUv");
	trackShader->linkProgram();

	simpleShadowFb = new myFramebuffer(viewPortWidth, viewPortHeight, GL_RED, GL_RED);

#ifdef Q_OS_LINUX
	simpleSMShader = new myShader(":/shaders/simpleSM.vert", ":/shaders/simpleSM.frag");
#endif
#ifdef Q_OS_WIN32
	simpleSMShader = new myShader(":/shaders/simpleSM.vert", ":/shaders/simpleSM.frag");
#endif
#ifdef Q_OS_MAC
	simpleSMShader = new myShader(":/shaders/simpleSM.vert", ":/shaders/simpleSM.frag");
#endif
	simpleSMShader->useAttribute(0, "aPosition");
	simpleSMShader->setOutput(0, "visibility");
	simpleSMShader->linkProgram();

	shadowVolumeFb = new myFramebuffer(viewPortWidth, viewPortHeight, GL_RED, GL_RED, true);
	shadowVolumeFb->setClearColor(0, 0, 0);

#ifdef Q_OS_LINUX
	shadowVolumeShader = new myShader(":/shaders/shadowVolume.vert", ":/shaders/shadowVolume.frag");
#endif
#ifdef Q_OS_WIN32
	shadowVolumeShader = new myShader(":/shaders/shadowVolume.vert", ":/shaders/shadowVolume.frag");
#endif
#ifdef Q_OS_MAC
	shadowVolumeShader = new myShader(":/shaders/shadowVolume.vert", ":/shaders/shadowVolume.frag");
#endif
	shadowVolumeShader->useAttribute(0, "aPosition");
	shadowVolumeShader->setOutput(0, "visibility");
	shadowVolumeShader->linkProgram();

	normalMapFb = new myFramebuffer(viewPortWidth, viewPortHeight, GL_RGBA16F, GL_RGBA, true);
	normalMapFb->setClearColor(0, 0, 0);

#ifdef Q_OS_LINUX
	normalMapShader = new myShader(":/shaders/normals.vert", ":/shaders/normals.frag");
#endif
#ifdef Q_OS_WIN32
	normalMapShader = new myShader(":/shaders/normals.vert", ":/shaders/normals.frag");
#endif
#ifdef Q_OS_MAC
	normalMapShader = new myShader(":/shaders/normals.vert", ":/shaders/normals.frag");
#endif
	normalMapShader->useAttribute(0, "aPosition");
	normalMapShader->useAttribute(7, "aNormal");
	normalMapShader->setOutput(0, "normal");
	normalMapShader->linkProgram();

	occlusionFb = new myFramebuffer(viewPortWidth, viewPortHeight, GL_RED, GL_RED);
	occlusionFb->setClearColor(0, 0, 0);

	preDistortionFb = new myFramebuffer(viewPortWidth, viewPortHeight, GL_RGBA, GL_RGBA, true);
	preDistortionFb->setClearColor(0, 0, 0);

#ifdef Q_OS_LINUX
	occlusionShader = new myShader(":/shaders/occlusion.vert", ":/shaders/occlusion.frag");
#endif
#ifdef Q_OS_WIN32
	occlusionShader = new myShader(":/shaders/occlusion.vert", ":/shaders/occlusion.frag");
#endif
#ifdef Q_OS_MAC
	occlusionShader = new myShader(":/shaders/occlusion.vert", ":/shaders/occlusion.frag");
#endif
	occlusionShader->useAttribute(0, "aPosition");
	occlusionShader->setOutput(0, "visibility");
	occlusionShader->linkProgram();

#ifdef Q_OS_LINUX
	debugShader = new myShader(":/shaders/debug.vert", ":/shaders/debug.frag");
#endif
#ifdef Q_OS_WIN32
	debugShader = new myShader(":/shaders/debug.vert", ":/shaders/debug.frag");
#endif
#ifdef Q_OS_MAC
	debugShader = new myShader(":/shaders/debug.vert", ":/shaders/debug.frag");
#endif
	debugShader->useAttribute(0, "aPosition");
	debugShader->linkProgram();

#ifdef USE_OVR
#ifdef Q_OS_LINUX
	oculusShader = new myShader("oculus.vert", "oculus.frag");
#endif
#ifdef Q_OS_WIN32
	oculusShader = new myShader("../FVD/oculus.vert", "../FVD/oculus.frag");
#endif
#ifdef Q_OS_MAC
	oculusShader = new myShader(":/shaders/oculus.vert", ":/shaders/oculus.frag");
#endif
	oculusShader->useAttribute(0, "aPosition");
	oculusShader->linkProgram();
#endif
}

void glViewWidget::moveCamera()
{
	if(povMode)
	{
		povPos += (int)(F_HZ*renderTime)*cameraBoost*(cameraMov.x - cameraMov.z);
		if(gloParent->curTrack())
		{
			if(povPos < 0)
			{
				povPos += gloParent->curTrack()->getNumPoints();
			}
			if(povPos > gloParent->curTrack()->getNumPoints())
			{
				povPos = 0;
			}
			povNode = gloParent->curTrack()->getPoint(povPos);
			if(povNode != NULL)
			{
				gloParent->mGraphWidget->drawGraph(10); // 10 = povPos
			}
			else    // should not be possible right now
			{
				povMode = false;
			}
		}
		else
		{
			povMode = false;
		}
	}

	if(moveMode) {
		float rotateX = 0.25f * (mousePos.x() - cursor().pos().x());
		float rotateY = 0.25f * (mousePos.y() - cursor().pos().y());
		float sign = 1.f;
		glm::vec3 up = glm::cross(freeFlySide, freeFlyDir);
		if(up.y < 0)
		{
			sign = -1.f;
		}
		if(!povMode)
		{
			freeFlyDir = glm::angleAxis(TO_RAD(rotateX), sign*glm::vec3(0, 1, 0))*freeFlyDir;
			freeFlySide = glm::angleAxis(TO_RAD(rotateX), sign*glm::vec3(0, 1, 0))*freeFlySide;
			freeFlyDir = glm::angleAxis(TO_RAD(rotateY), freeFlySide)*freeFlyDir;
			//freeFlyDir = glm::vec3(glm::rotate(, glm::cross(freeFlyDir, glm::vec3(0.f, 1.f, 0.f))) * glm::vec4(freeFlyDir, 0.f));
		}
		this->cursor().setPos(mousePos);
	}

	if(!povMode)
	{
		freeFlyPos += 0.3f*cameraBoost*(cameraMov.x - cameraMov.z)*freeFlyDir - 0.3f*cameraBoost*(cameraMov.y - cameraMov.w)*glm::normalize(glm::cross(freeFlyDir, glm::vec3(0.f, 1.f, 0.f)));
		freeFlyPos += 0.01f*cameraJump * cameraBoost * glm::cross(freeFlyDir, glm::normalize(glm::cross(freeFlyDir, glm::vec3(0.f, 1.f, 0.f))));
	}
#ifdef USE_OVR
	Quatf bla;
	if(sensFusion)
	{
		bla = sensFusion->GetOrientation();
	}

	bla.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&headPos.z, &headPos.y, &headPos.x);
#endif
	cameraJump = 0;
}

void glViewWidget::buildMatrices(float offset)
{
	float scew = 0;//offset*FNEAR/screenDist;
	float side = tan(fov*F_PI/360.)*(FNEAR);
	glm::mat4 anchorBase;
	if(riftMode/* && gloParent->isFullScreen()*/)
	{
		//Matrix4f bla = Matrix4f::PerspectiveRH(2*atan(vScreenSize/(2*fEyeToScreen)), viewPortWidth/(2.f*viewPortHeight), FNEAR, FFAR);
		float fov = OCULUS_SCALE*vScreenSize/(2*fEyeToScreen);
		float aspect = viewPortWidth/(2.f*viewPortHeight);
		ProjectionMatrix = glm::frustum(-aspect*fov*FNEAR, aspect*fov*FNEAR, -fov*FNEAR, fov*FNEAR, FNEAR, FFAR);

		float h = (offset/fabs(offset)) *(hScreenSize - lensSep*2.f)/hScreenSize;
		if(h != h) h = 0.f;
		ProjectionMatrix = glm::translate(glm::vec3(h, 0.f, 0.f)) * ProjectionMatrix;
	}
	else
	{
		ProjectionMatrix = glm::frustum(-side+scew, side+scew, -(float)viewPortHeight/viewPortWidth*side, (float)viewPortHeight/viewPortWidth*side, FNEAR, FFAR);
	}
	if(povMode)
	{
		anchorBase = glm::translate(gloParent->curTrack()->startPos) * glm::rotate(TO_RAD(gloParent->curTrack()->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));
		glm::vec3 pos = povNode->vRelPos(gloParent->curTrack()->povPos.y, gloParent->curTrack()->povPos.x);
		glm::vec3 direction = povNode->vDirHeart(gloParent->curTrack()->fHeart);
		glm::vec3 front = direction;
		glm::vec3 side = povNode->vLatHeart(gloParent->curTrack()->fHeart);
		glm::vec3 down = glm::cross(front, side);
		direction = glm::angleAxis((float)(-headPos.z), down) * glm::angleAxis((float)(headPos.y*180.f/F_PI), side) * direction;
		side = glm::angleAxis((float)(-headPos.z), down) * side;
		down = glm::cross(direction, side);
		down = glm::angleAxis((float)(-headPos.x), direction) * down;

		ModelMatrix = glm::lookAt(glm::vec3(anchorBase * glm::vec4(pos, 1.f)), glm::vec3(anchorBase * glm::vec4(pos+direction, 1.f)), -glm::vec3(anchorBase * glm::vec4(down, 0.f)));
		cameraPos = glm::vec3(anchorBase * glm::vec4(pos, 1.f));
		cameraDir = glm::vec3(anchorBase * glm::vec4(povNode->vDirHeart(gloParent->curTrack()->fHeart), 0.f));
	}
	else
	{
		cameraPos = freeFlyPos;
		glm::vec3 pos = cameraPos + offset*freeFlySide;

		ModelMatrix = glm::lookAt(pos, pos+freeFlyDir, glm::cross(freeFlySide, freeFlyDir));
		ProjectionModelMatrix = ProjectionMatrix*ModelMatrix;
	}

	ProjectionModelMatrix = ProjectionMatrix*ModelMatrix;
}

void glViewWidget::updateLoD()
{
	static int curTrack = 0;
	QList<trackHandler*> trackList = gloParent->getTrackList();
	if(trackList.isEmpty()) return;
	if(curTrack >= trackList.size()) curTrack = 0;

	curTrack = (curTrack+1)%trackList.size();
	trackList[curTrack]->mMesh->createIndices();
	return;
}

void glViewWidget::legacyDrawFloor()
{
	glColor4f(0.7f, 0.7f, 0.7f, 1.f);
	glBegin(GL_POLYGON);
	glVertex3f(-220.f, -1.f, -220.f);
	glVertex3f(220.f, -1.f, -220.f);
	glVertex3f(220.f, -1.f, 220.f);
	glVertex3f(-220.f, -1.f, 220.f);
	glEnd();

	glColor4f(0.3f, 0.3f, 0.3f, 0.3f);


	if(!gloParent->mOptions->drawGrid) return;
	for(int i = -210; i <= 210; i+=10)
	{
		glBegin(GL_LINES);
		glVertex3f((float)i, 0.0, -220.0);
		glVertex3f((float)i, 0.0, 220.0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(-220.0, 0.0, (float)i);
		glVertex3f(220.0, 0.0, (float)i);
		glEnd();
	}

}

void glViewWidget::legacyDrawTrack(trackHandler *_track)
{
	if(_track->trackData->drawHeartline == 2) return;

	track* myTrack = _track->trackData;
	glm::mat4 anchorBase = glm::translate(myTrack->startPos) * glm::rotate(TO_RAD(myTrack->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));
	glm::vec4 curPos;

	float meshQuality;
	switch(gloParent->mOptions->meshQuality)
	{
	case 0:
		meshQuality = 1;
		break;
	case 1:
		meshQuality = 2;
		break;
	case 2:
		meshQuality = 4;
		break;
	case 3:
		meshQuality = 6;
		break;
	}
	float minNodeDist = 12.f/(meshQuality), maxNodeDist = 0.3f/(meshQuality);    // minimal and maximal distance between nodes
	float angleNodeDist = 6.f/(meshQuality);                     // after x degrees difference force a new node

	// Spine
	float fSpine = myTrack->fHeart < 0 ? -0.3f : 0.3f;
	for(int i = 0; i < myTrack->lSections.size(); i++)
	{
		section* curSection = myTrack->lSections[i];
		if(curSection == myTrack->activeSection)
		{
			glColor3f(_track->trackColors[1].red()/255.f, _track->trackColors[1].green()/255.f , _track->trackColors[1].blue()/255.f);
		}
		else
		{
			glColor3f(_track->trackColors[0].red()/255.f, _track->trackColors[0].green()/255.f , _track->trackColors[0].blue()/255.f);
		}
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < curSection->lNodes.size(); ++j)
		{
			float distFromLastNode = 1.f;
			float angle = curSection->lNodes[j].fFlexion();
			angle /= angleNodeDist;
			angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
			angle *= curSection->lNodes[j].fDistFromLast;;
			distFromLastNode += angle;
			if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
			{
				if(distFromLastNode >= 1.f)
				{
					distFromLastNode -= 1.f;
				}
				else
				{
					distFromLastNode = 0.f;
				}

				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart+fSpine), 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
			}
		}
		int end = curSection->lNodes.size()-1;
		curPos = anchorBase * glm::vec4(curSection->lNodes[end].vPosHeart(myTrack->fHeart+fSpine), 1.f);
		glVertex3f(curPos.x, curPos.y, curPos.z);
		glEnd();
	}

	// Shadow
	glColor3f(0.0f, 0.0f , 0.0f);
	for(int i = 0; i < myTrack->lSections.size(); i++)
	{
		section* curSection = myTrack->lSections[i];
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < curSection->lNodes.size(); ++j)
		{
			float distFromLastNode = 1.f;
			float angle = curSection->lNodes[j].fFlexion();
			angle /= angleNodeDist;
			angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
			angle *= curSection->lNodes[j].fDistFromLast;;
			distFromLastNode += angle;
			if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
			{
				if(distFromLastNode >= 1.f)
				{
					distFromLastNode -= 1.f;
				}
				else
				{
					distFromLastNode = 0.f;
				}

				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart+fSpine), 1.f);
				glVertex3f(curPos.x, 0.f, curPos.z);
			}
		}
		int end = curSection->lNodes.size()-1;
		curPos = anchorBase * glm::vec4(curSection->lNodes[end].vPosHeart(myTrack->fHeart+fSpine), 1.f);
		glVertex3f(curPos.x, 0.f, curPos.z);
		glEnd();
	}


	// right Track
	for(int i = 0; i < myTrack->lSections.size(); i++)
	{
		section* curSection = myTrack->lSections[i];
		if(curSection == myTrack->activeSection)
		{
			glColor3f(_track->trackColors[1].red()/255.f, _track->trackColors[1].green()/255.f , _track->trackColors[1].blue()/255.f);
		}
		else
		{
			glColor3f(_track->trackColors[0].red()/255.f, _track->trackColors[0].green()/255.f , _track->trackColors[0].blue()/255.f);
		}
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < curSection->lNodes.size(); ++j)
		{
			float distFromLastNode = 1.f;
			float angle = curSection->lNodes[j].fFlexion();
			angle /= angleNodeDist;
			angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
			angle *= curSection->lNodes[j].fDistFromLast;;
			distFromLastNode += angle;
			if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
			{
				if(distFromLastNode >= 1.f)
				{
					distFromLastNode -= 1.f;
				}
				else
				{
					distFromLastNode = 0.f;
				}

				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart)+curSection->lNodes[j].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
			}
		}
		int end = curSection->lNodes.size()-1;
		curPos = anchorBase * glm::vec4(curSection->lNodes[end].vPosHeart(myTrack->fHeart)+curSection->lNodes[end].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
		glVertex3f(curPos.x, curPos.y, curPos.z);
		glEnd();
	}



	// left Track
	for(int i = 0; i < myTrack->lSections.size(); i++)
	{
		section* curSection = myTrack->lSections[i];
		if(curSection == myTrack->activeSection)
		{
			glColor3f(_track->trackColors[1].red()/255.f, _track->trackColors[1].green()/255.f , _track->trackColors[1].blue()/255.f);
		}
		else
		{
			glColor3f(_track->trackColors[0].red()/255.f, _track->trackColors[0].green()/255.f , _track->trackColors[0].blue()/255.f);
		}
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < curSection->lNodes.size(); ++j)
		{
			float distFromLastNode = 1.f;
			float angle = curSection->lNodes[j].fFlexion();
			angle /= angleNodeDist;
			angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
			angle *= curSection->lNodes[j].fDistFromLast;;
			distFromLastNode += angle;
			if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
			{
				if(distFromLastNode >= 1.f)
				{
					distFromLastNode -= 1.f;
				}
				else
				{
					distFromLastNode = 0.f;
				}

				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart)-curSection->lNodes[j].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
			}
		}
		int end = curSection->lNodes.size()-1;
		curPos = anchorBase * glm::vec4(curSection->lNodes[end].vPosHeart(myTrack->fHeart)-curSection->lNodes[end].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
		glVertex3f(curPos.x, curPos.y, curPos.z);
		glEnd();
	}


	float distFromLastX = 0.0;
	// crossties
	for(int i = 0; i < myTrack->lSections.size(); i++)
	{
		section* curSection = myTrack->lSections[i];
		if(curSection == myTrack->activeSection)
		{
			glColor3f(_track->trackColors[1].red()/255.f, _track->trackColors[1].green()/255.f , _track->trackColors[1].blue()/255.f);
		}
		else
		{
			glColor3f(_track->trackColors[0].red()/255.f, _track->trackColors[0].green()/255.f , _track->trackColors[0].blue()/255.f);
		}
		for(int j = 0; j < curSection->lNodes.size(); j++)
		{
			distFromLastX += curSection->lNodes[j].fDistFromLast;
			if(distFromLastX >= 1.0f)
			{
				distFromLastX -= 1.0f;
				glBegin(GL_LINE_STRIP);
				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart)-curSection->lNodes[j].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart+fSpine), 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
				curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPosHeart(myTrack->fHeart)+curSection->lNodes[j].vLatHeart(myTrack->fHeart)*0.5f, 1.f);
				glVertex3f(curPos.x, curPos.y, curPos.z);
				glEnd();
			}
		}
	}

	if(_track->trackData->drawHeartline != 1)
	{
		for(int i = 0; i < myTrack->lSections.size(); i++)
		{
			section* curSection = myTrack->lSections[i];
			if(curSection == myTrack->activeSection)
			{
				glColor3f(1.0f, 1.0f , 1.0f);
			}
			else
			{
				glColor3f(0.9f, 0.9f , 0.0f);
			}
			glBegin(GL_LINE_STRIP);
			for(int j = 0; j < curSection->lNodes.size(); ++j)
			{
				float distFromLastNode = 1.f;
				float angle = curSection->lNodes[j].fFlexion();
				angle /= angleNodeDist;
				angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
				angle *= curSection->lNodes[j].fDistFromLast;;
				distFromLastNode += angle;
				if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
				{
					if(distFromLastNode >= 1.f)
					{
						distFromLastNode -= 1.f;
					}
					else
					{
						distFromLastNode = 0.f;
					}

					curPos = anchorBase * glm::vec4(curSection->lNodes[j].vPos, 1.f);
					glVertex3f(curPos.x, curPos.y, curPos.z);
				}
			}
			int end = curSection->lNodes.size()-1;
			curPos = anchorBase * glm::vec4(curSection->lNodes[end].vPos, 1.f);
			glVertex3f(curPos.x, curPos.y, curPos.z);
			glEnd();
		}
	}
}
