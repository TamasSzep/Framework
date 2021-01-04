// EngineBuildingBlocks/Graphics/Camera/FreeCamera.cpp

#include <EngineBuildingBlocks/Graphics/Camera/FreeCamera.h>

#include <EngineBuildingBlocks/SystemTime.h>
#include <EngineBuildingBlocks/ErrorHandling.h>
#include <EngineBuildingBlocks/Input/KeyHandler.h>
#include <EngineBuildingBlocks/Input/MouseHandler.h>

#include <cmath>

using namespace EngineBuildingBlocks;
using namespace EngineBuildingBlocks::Graphics;
using namespace EngineBuildingBlocks::Input;

FreeCamera::FreeCamera(SceneNodeHandler* sceneNodeHandler,
	KeyHandler* keyHandler, MouseHandler* mouseHandler)
	: Camera(sceneNodeHandler)
{
	Initialize(keyHandler, mouseHandler);
}

FreeCamera::FreeCamera(SceneNodeHandler* sceneNodeHandler, 
	KeyHandler* keyHandler, MouseHandler* mouseHandler, const glm::vec3& position, const glm::vec3& direction,
	float fovY, float aspectRatio, float nearPlaneDistance, float farPlaneDistance, bool isProjectingTo_0_1_Interval)
	: Camera(sceneNodeHandler)
{
	Camera::SetLocation(position, direction);
	Camera::SetPerspectiveProjection(fovY, aspectRatio, nearPlaneDistance, farPlaneDistance,
		isProjectingTo_0_1_Interval);

	Initialize(keyHandler, mouseHandler);
}

FreeCamera::FreeCamera(SceneNodeHandler* sceneNodeHandler, 
	KeyHandler* keyHandler, MouseHandler* mouseHandler, const glm::vec3& position, const glm::vec3& direction,
	float left, float right, float bottom, float top, float nearPlaneDistance, float farPlaneDistance, bool isProjectingTo_0_1_Interval)
	: Camera(sceneNodeHandler)
{
	Camera::SetLocation(position, direction);
	Camera::SetPerspectiveProjection(left, right, bottom, top, nearPlaneDistance, farPlaneDistance,
		isProjectingTo_0_1_Interval);

	Initialize(keyHandler, mouseHandler);
}

FreeCamera::~FreeCamera()
{
}

void FreeCamera::Initialize(KeyHandler* keyHandler, MouseHandler* mouseHandler)
{
	m_MouseDifference = glm::vec2(0.0f);
	m_KeyDownTime = m_SpeedTime = glm::vec3(0.0f);
	m_IsHandlingEvents = true;

	m_MaxSpeed = 5.0f;
	m_MaxSpeedAccelleration = 10.0f;
	m_AccellerationTime = 0.2f;
	m_MouseSensitivity = 0.002f;

	// Registering for key events.
	if (keyHandler != nullptr)
	{
		auto eventClassIds = keyHandler->RegisterTimedKeyEventListener(
			{
				"FreeCamera.MoveForward",
				"FreeCamera.MoveBackward",
				"FreeCamera.MoveUp",
				"FreeCamera.MoveDown",
				"FreeCamera.MoveLeft",
				"FreeCamera.MoveRight",
				"FreeCamera.IncreaseMoveSpeed",
				"FreeCamera.DecreaseMoveSpeed"
			}, this);

		KeyEventName_MoveForward_ECI = eventClassIds[0];
		KeyEventName_MoveBackward_ECI = eventClassIds[1];
		KeyEventName_MoveUp_ECI = eventClassIds[2];
		KeyEventName_MoveDown_ECI = eventClassIds[3];
		KeyEventName_MoveLeft_ECI = eventClassIds[4];
		KeyEventName_MoveRight_ECI = eventClassIds[5];
		KeyEventName_IncreaseMoveSpeed_ECI = eventClassIds[6];
		KeyEventName_DecreaseMoveSpeed_ECI = eventClassIds[7];
	}

	// Registering for mouse events.
	if (mouseHandler != nullptr)
	{
		MouseEventName_Rotate_ECI = mouseHandler->RegisterMouseDragEventListener("FreeCamera.Rotate", this);
	}
}

float FreeCamera::GetMaxSpeed() const
{
	return m_MaxSpeed;
}

void FreeCamera::SetMaxSpeed(float maxSpeed)
{
	this->m_MaxSpeed = maxSpeed;
}

float FreeCamera::GetMaxSpeedAccelleration() const
{
	return m_MaxSpeedAccelleration;
}

void FreeCamera::SetMaxSpeedAccelleration(float acceleration)
{
	this->m_MaxSpeedAccelleration = acceleration;
}

float FreeCamera::GetAccellerationTime() const
{
	return m_AccellerationTime;
}

void FreeCamera::SetAccellerationTime(float time)
{
	this->m_AccellerationTime = time;
}

float FreeCamera::GetMouseSensitivity() const
{
	return m_MouseSensitivity;
}

void FreeCamera::SetMouseSensitivity(float sensitivity)
{
	this->m_MouseSensitivity = sensitivity;
}

void FreeCamera::SetHandlingEvents(bool isHandlingEvents)
{
	this->m_IsHandlingEvents = isHandlingEvents;
}

bool FreeCamera::HandleEvent(const Event* _event)
{
	if (!m_IsHandlingEvents)
	{
		return false;
	}

	unsigned eventClassId = _event->ClassId;
	
	if (eventClassId == KeyEventName_MoveForward_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.x += dt;
	}
	else if (eventClassId == KeyEventName_MoveBackward_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.x -= dt;
	}
	else if (eventClassId == KeyEventName_MoveUp_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.y += dt;
	}
	else if (eventClassId == KeyEventName_MoveDown_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.y -= dt;
	}
	else if (eventClassId == KeyEventName_MoveLeft_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.z -= dt;
	}
	else if (eventClassId == KeyEventName_MoveRight_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_KeyDownTime.z += dt;
	}
	else if (eventClassId == KeyEventName_IncreaseMoveSpeed_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_MaxSpeed += m_MaxSpeedAccelleration * dt;
	}
	else if (eventClassId == KeyEventName_DecreaseMoveSpeed_ECI)
	{
		float dt = static_cast<float>(ToKeyEvent(_event).TotalDownTime);
		m_MaxSpeed = std::max(m_MaxSpeed - m_MaxSpeedAccelleration * dt, 0.0f);
	}
	else if (eventClassId == MouseEventName_Rotate_ECI)
	{
		m_MouseDifference += ToMouseDragEvent(_event).DragDifference;
	}
	else
	{
		RaiseException("Unknown event was received.");
	}

	return true;
}

void FreeCamera::UpdateRotation()
{
	if (m_MouseDifference == glm::vec2(0.0f)) return;

	auto worldTransformation = GetWorldTransformation();
	auto& direction = worldTransformation.A[0];
	auto& right = worldTransformation.A[2];

	float wUpAngle = -m_MouseDifference.x * m_MouseSensitivity;
	float rightAngle = -m_MouseDifference.y * m_MouseSensitivity;

	m_MouseDifference = glm::vec2(0.0f);

	glm::mat4 rotation = glm::rotate(wUpAngle, SceneNodeHandler::c_WorldUp)
		* glm::rotate(rightAngle, right);
	glm::vec4 newDirection = rotation * glm::vec4(direction, 0.0f);

	// Preventing camera flip.
	{
		const float yLimit = 0.99f;

		newDirection = glm::normalize(newDirection);
		float currentXZLengthSqr = newDirection.x * newDirection.x + newDirection.z * newDirection.z;
		float requiredXZLengthSqr = 1.0f - yLimit * yLimit;
		float ratio = sqrtf(requiredXZLengthSqr / currentXZLengthSqr);
		if (newDirection.y < -yLimit)
		{
			newDirection.y = -yLimit;
			newDirection.x *= ratio;
			newDirection.z *= ratio;
		}
		if (newDirection.y > yLimit)
		{
			newDirection.y = yLimit;
			newDirection.x *= ratio;
			newDirection.z *= ratio;
		}
	}

	SetDirection(glm::vec3(newDirection.x, newDirection.y, newDirection.z));
}

void FreeCamera::UpdateTranslation(const SystemTime& systemTime)
{
	float dt = static_cast<float>(systemTime.GetElapsedTime());

	auto speed = glm::vec3(
		GetSpeed(m_SpeedTime.x, m_KeyDownTime.x, dt),
		GetSpeed(m_SpeedTime.y, m_KeyDownTime.y, dt),
		GetSpeed(m_SpeedTime.z, m_KeyDownTime.z, dt));

	m_KeyDownTime = glm::vec3(0.0f);

	auto worldTransformation = GetWorldTransformation();
	auto& position = worldTransformation.Position;
	auto& direction = worldTransformation.A[0];
	auto& up = worldTransformation.A[1];
	auto& right = worldTransformation.A[2];

	if (speed == glm::vec3(0.0f))
	{
		return;
	}

	position += (speed.x * direction + speed.y * up + speed.z * right) * dt;

	SetPosition(position);
}

float FreeCamera::GetSpeed(float& speedTime, float keyDownTime, float dt)
{
	if (keyDownTime == 0.0f) // Either no key pressing or both keys pressed for the exact same (probably whole) time.
	{
		if (speedTime >= 0.0) speedTime = std::max(speedTime - dt, 0.0f);
		else speedTime = std::min(speedTime + dt, 0.0f);
	}
	else
	{
		speedTime = glm::clamp(speedTime + keyDownTime, -m_AccellerationTime, m_AccellerationTime);
	}

	return m_MaxSpeed * speedTime / m_AccellerationTime;
}

void FreeCamera::Update(const SystemTime& systemTime)
{
	UpdateRotation();
	UpdateTranslation(systemTime);
}