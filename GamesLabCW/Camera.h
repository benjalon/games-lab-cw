class Camera
{
private:
	glm::vec3 scale;
	glm::vec3 worldPos;
	glm::vec3 rotate;
public:
	Camera();
	~Camera();

	void SetScale(float ScaleX, float ScaleY, float ScaleZ);
	void SetWorldPos(float x, float y, float z);
	void SetRotate(float RotateX, float RotateY, float RotateZ);
	const glm::mat4* GetTrans();
};

