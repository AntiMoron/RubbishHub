#include<stdio.h>
#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<algorithm>
#include<string.h>
#include<type_traits>
using namespace std;

bool read_int(int& val,FILE*& fp)
{
	if(fp == nullptr)
		return false;
	fread(&val,sizeof(int),1,fp);
	return true;
}

bool write_int(int val,FILE*& fp)
{
	if(fp == nullptr)
	{
		return false;
	}
	fwrite(&val,sizeof(int),1,fp);
	return true;
}

struct SANHeader
{
	char CEK[4];
};


struct SANOBJ
{
	char path[128];
	char nickname[40];
	SANOBJ()
	{
		memset(path,0,sizeof(path));
		memset(nickname,0,sizeof(nickname));
	}
	SANOBJ(const char* _p,const char* _n)
	{
		int m = strlen(_p),n = strlen(_n);
		if(m < 128) strcpy(path,_p);
		if(n < 40) strcpy(nickname,_n);
	}
	SANOBJ(const SANOBJ& other)
	{
		memcpy(path,other.path,sizeof(char) * 128);
		memcpy(nickname,other.nickname,sizeof(char) * 40);
	}
	bool operator < (const SANOBJ& other) const
	{
		return string(path) < string(other.path);
	}
	bool operator == (const SANOBJ& other) const
	{
		return (strcmp(other.path,path) == 0);
	}
};

struct SANAutoPicture
{
	char path[128];
	SANAutoPicture()
	{
		memset(path,0,sizeof(path));
	}
	SANAutoPicture(const SANAutoPicture& other)
	{
		memcpy(path,other.path,sizeof(char) * 128);
	}
	bool operator == (const SANAutoPicture& other) const
	{
		return (strcmp(path,other.path) == 0);
	}
	bool operator < (const SANAutoPicture& other)const
	{
		return string(path) < string(other.path);
	}
};

struct SANClickboxQuestionId
{
	int id;
	SANClickboxQuestionId(int _id = 0):id(_id){}
	bool operator == (const SANClickboxQuestionId& other)const
	{
		return id == other.id;
	}
	bool operator < (const SANClickboxQuestionId& other)
	{
		return id < other.id;
	}
};

struct SANAudio
{
	char path[128];
	SANAudio(){memset(path,0,sizeof(path));}
	SANAudio(const SANAudio& other){memcpy(path,other.path,sizeof(char) * 128);}
	bool operator == (const SANAudio& other)const
	{
		return (strcmp(path,other.path) == 0);
	}
	bool operator < (const SANAudio& other)const
	{
		return string(path) < string(other.path);
	}
};

template <typename source_type>		//the 'source_type' type need to have the member 'int m_index'
class FrameQueue
{
public:
	FrameQueue()	//fill the 1st frame automatically
	{
		source_type new_node;
		new_node.m_index = 0;
		m_data.push_back(new_node);
	}
	FrameQueue(const FrameQueue& other)
	{
		m_data = other.m_data;
	}

	bool AddFrame(const source_type& other)	// keeps an ascending order
	{
		int index = _binary_search(other);
		if(index != -1)
		{
			return false;
		}
		m_data.insert(std::upper_bound(m_data.begin(),m_data.end(),other,
									[](const source_type& a,const source_type& b)->bool const{return a.m_index < b.m_index;}
									),other);
		return true;
	}

	bool DeleteFrameByElemIndex(int elemIndex)	//delete frame according to the index of frame in the queue
	{
		if(elemIndex < 0)
			return false;
		if(elemIndex >= m_data.size())
			return false;
		typename std::vector<source_type>::iterator it ;
		it = m_data.begin() + elemIndex;
		it = m_data.erase(it);
		return true;
	}

	bool DeleteFrameByFrameIndex(int frameIndex)
	{
		source_type node = {};
		node.m_index = frameIndex;
		int index = _binary_search(node);
		if(index == -1)
		{
			return false;
		}
		typename std::vector<source_type>::iterator it;
		it = m_data.begin() + index;
		it = m_data.erase(it);
		return true;
	}

	bool Clear()		// There would always be a single frame
	{
		source_type new_node = {};
		new_node.m_index = 0;
		m_data.clear();
		m_data.push_back(new_node);
		return true;
	}

	bool WriteFile(FILE*& fp)
	{
		if(fp == nullptr)
			return false;
		bool result = write_int(m_data.size(),fp);
		if(result == false)
			return false;
		fwrite(&(m_data[0]),sizeof(source_type),m_data.size(),fp);
		return true;
	}

	bool ReadFile(FILE*& fp)
	{
		if(fp == nullptr)
			return false;
		int data_size;
		bool result = read_int(data_size,fp);
		if(result == false)
			return false;
		if(data_size > 0)
		{
			m_data.resize(data_size);
			fread(&(m_data[0]),sizeof(source_type),data_size,fp);
		}
		return true;
	}
private:
	int _binary_search(source_type target)
	{
		int l = 0,r = (int)m_data.size() - 1,mid;
		while(l<=r)
		{
			mid = (l + r) / 2;
			if(m_data[l].m_index == target.m_index)
			{
				return l;
			}
			if(m_data[r].m_index == target.m_index)
			{
				return r;
			}
			if(m_data[mid].m_index == target.m_index)
			{
				return mid;
			}
			if(m_data[mid].m_index > target.m_index)
			{
				r = mid - 1;
			}
			else
			{
				l = mid + 1;
			}
		}
		return -1;
	}
	public:
	vector<source_type> m_data;
};

template<typename source_type>
class UniqueSource
{
public:
	UniqueSource(){}
	~UniqueSource(){}
	bool Add(const source_type& other)//return false when insert failed,otherwise return true
	{
		if(m_map_source_to_index.find(other) == m_map_source_to_index.end())
		{
			int map_size = m_map_source_to_index.size();
			m_data.push_back(other);
			m_map_source_to_index.insert(pair<source_type,int>(other,map_size));
			m_result.push_back(map_size);
			return true;
		}
		else
		{
			m_result.push_back(m_map_source_to_index[other]);
			return true;
		}
		return false;
	}

	bool Delete(int elemIndex)	// delete the elem by elem Index,If succeed ,return true,otherwise return false
	{
		if(elemIndex < 0)
			return false;
		if(elemIndex >= m_data.size())
			return false;
		typename std::map<source_type,int>::iterator mit;
		typename std::vector<source_type>::iterator vit;
		for(mit = m_map_source_to_index.begin();mit!=m_map_source_to_index.end();++mit)
		{
			mit = m_map_source_to_index.erase(mit);
		}
		vit = m_data.begin() + elemIndex;
		vit = m_data.erase(vit);
		return true;
	}

	bool Clear()
	{
		m_map_source_to_index.clear();
		m_data.clear();
		m_result.clear();
		return true;
	}

	bool WriteFile(FILE*& fp)
	{
		if(fp == nullptr)
			return false;
		bool result = write_int(m_data.size(),fp);
		if(result == false)
			return false;
		if(m_data.size() > 0)
			fwrite(&(m_data[0]),sizeof(source_type),m_data.size(),fp);

		result = write_int(m_result.size(),fp);
		if(result == false)
			return false;
		if(m_result.size() > 0)
			fwrite(&(m_result[0]),sizeof(int),m_result.size(),fp);
		return true;
	}

	bool ReadFile(FILE*& fp)
	{
		if(fp == nullptr)
			return false;
		Clear();
		int data_size;
		read_int(data_size,fp);
		if(data_size > 0)
		{
			m_data.resize(data_size);
			fread(&(m_data[0]),sizeof(source_type),data_size,fp);
		}
		read_int(data_size,fp);
		if(data_size > 0)
		{
			m_result.resize(data_size);
			fread(&(m_result[0]),sizeof(int),data_size,fp);
		}

		for(int i=0;i<m_data.size();i++)
		{
			m_map_source_to_index.insert(pair<source_type,int>(m_data[i],i));
		}
		return true;
	}
//private:
	map<source_type,int> m_map_source_to_index;
	vector<source_type> m_data;
	vector<int> m_result;	//the index I want
};

struct filestructure
{
	typedef struct tagFrontLight{int m_index; int m_lightState;/*true for light on,otherwise off*/} tagFrontLight;	//远光灯
	typedef struct tagTurnLight{int m_index; int m_lightState;/*-1 for left,0 for nothing,1 for right*/} tagTurnLight;			//转向灯
	typedef struct tagWiper{int m_index; int m_wiperState; /*1 for on,0 for off*/} tagWiper;		//雨刷器
	typedef struct tagAutoPicture{int m_index;int m_picIndex; int m_posx,m_posy,m_lenx,m_leny,m_startFrame,m_endFrame;} tagAutoPicture;	// 自动弹出图片
	typedef struct tagAutoQuestion{int m_index; int m_quesIndex; int m_frameIndex,m_questionId;} tagAutoQuestion;	//自动弹出问题
	typedef struct tagAudio{int m_index; int m_audioIndex; int m_startFrame;} tagAudio;					//声音
	typedef struct tagFog{int m_index; float m_fogColor[4]; float m_startDistance;float m_endDistance;} tagFog;						//雾气
	typedef struct tagCamera{int m_index;float m_position[3]; float m_rotation[3];} tagCamera;				//摄像机
	typedef struct tagModel{int m_index; float m_position[3]; float m_rotation[3]; float m_scaling[3];} tagModel;					//模型
	typedef struct tagClickBox{int m_index; float m_position[3]; float m_rotation[3]; float m_scaling[3];} tagClickBox;			//点击盒

public:
	filestructure(){}
	~filestructure(){}

	bool AddModel(const char* nickname,const char* filename)
	{
		SANOBJ new_node = {};
		memcpy(new_node.nickname,nickname,sizeof(char) * 40);
		memcpy(new_node.path,filename,sizeof(char) * 128);
		m_obj_source.Add(new_node);
		return true;
	}

	bool AddPickbox(int question_id)
	{
		SANClickboxQuestionId;
		return true;
	}

	bool AddFrame(int layerIndex,const tagFrontLight& other)
	{
		if(layerIndex != 0)
			return false;
		m_frontLightDisplay.AddFrame(other);
		return true;
	}
	bool AddFrame(int layerIndex,const tagTurnLight& other)
	{
		if(layerIndex != 1)
			return false;
		m_turnLight.AddFrame(other);
		return true;
	}
	bool AddFrame(int layerIndex,const tagWiper& other)
	{
		if(layerIndex != 2)
			return false;
		m_wiper.AddFrame(other);
		return true;
	}

	bool AddFrame(int layerIndex,const tagAutoPicture& other)
	{
		if(layerIndex != 3)
		{
			return false;
		}
		m_autoPicture.AddFrame(other);
		return true;
	}
	bool AddFrame(int layerIndex,const tagAutoQuestion& other)
	{
		if(layerIndex != 4)
		{
			return false;
		}
		m_autoQuestion.AddFrame(other);
		return true;
	}

	bool AddFrame(int layerIndex,const tagAudio& other)
	{
		if(layerIndex != 5)
		{
			return false;
		}
		m_audio.AddFrame(other);
		return true;
	}
	bool AddFrame(int layerIndex,const tagFog& other)
	{
		if(layerIndex != 6)
		{
			return false;
		}
		m_fog.AddFrame(other);
		return true;
	}

	bool AddFrame(int layerIndex,const tagCamera& other)
	{
		if(layerIndex != 7)
		{
			return false;
		}
		m_camera.AddFrame(other);
		return true;
	}

	bool AddFrame(int layerIndex,const tagModel& other)
	{
		if(layerIndex < 8)
		{
			return false;
		}
		if(layerIndex >= m_models.size() + 8)
		{
			return false;
		}
		m_models[layerIndex - 8].AddFrame(other);
		return true;
	}

	bool AddFrame(int layerIndex,const tagClickBox& other)
	{
		if(layerIndex < 8 + m_models.size())
		{
			return false;
		}
		if(layerIndex >= m_models.size() + m_click.size() + 8)
		{
			return false;
		}
		m_click[layerIndex - m_models.size() - 8].AddFrame(other);
		return true;
	}

	bool WriteFile(const char* filename)
	{
//@Unfinished
		FILE* fp = nullptr;
		fp = fopen(filename,"wb");
		if(fp == nullptr)
			return false;
		m_obj_source.WriteFile(fp);
		m_autoPicture_source.WriteFile(fp);
		m_clickBoxQuestionId_source.WriteFile(fp);
		m_audio_source.WriteFile(fp);

		m_turnLight.WriteFile(fp);
		m_wiper.WriteFile(fp);
		m_autoPicture.WriteFile(fp);
		m_autoQuestion.WriteFile(fp);
		m_audio.WriteFile(fp);
		m_fog.WriteFile(fp);
		m_camera.WriteFile(fp);

		write_int(m_models.size(),fp);
		for(int i=0;i<m_models.size();i++)
		{
			m_models[i].WriteFile(fp);
		}
		write_int(m_click.size(),fp);
		for(int i=0;i<m_click.size();i++)
		{
			m_click[i].WriteFile(fp);
		}
		fclose(fp);
		return true;
	}

	bool ReadFile(const char* filename)
	{
//@Unfinished
		int modelNumber;
		int clickNumber;
		FILE* fp = nullptr;
		fp = fopen(filename,"rb");
		if(fp == nullptr)
			return false;
		Clear();
		m_obj_source.ReadFile(fp);
		m_autoPicture_source.ReadFile(fp);
		m_clickBoxQuestionId_source.ReadFile(fp);
		m_audio_source.ReadFile(fp);

		m_frontLightDisplay.ReadFile(fp);
		m_turnLight.ReadFile(fp);
		m_wiper.ReadFile(fp);
		m_autoPicture.ReadFile(fp);
		m_autoQuestion.ReadFile(fp);
		m_audio.ReadFile(fp);
		m_fog.ReadFile(fp);
		m_camera.ReadFile(fp);
		read_int(modelNumber,fp);
		m_models.resize(modelNumber);
		for(int i=0;i<modelNumber;i++)
			m_models.ReadFile(fp);
		read_int(clickNumber,fp);
		m_click.resize(clickNumber);
		for(int i=0;i<m_clickNumber;i++)
			m_click.ReadFile(fp);
		fclose(fp);
		return true;
	}

	bool Render()
	{
		printf("%d\r\n");
		return true;
	}
private:
// the resouce heap
	UniqueSource<SANOBJ> m_obj_source;
	UniqueSource<SANAutoPicture> m_autoPicture_source;
	UniqueSource<SANClickboxQuestionId> m_clickBoxQuestionId_source;
	UniqueSource<SANAudio> m_audio_source;

// the display heap
	FrameQueue<tagFrontLight> m_frontLightDisplay;
	FrameQueue<tagTurnLight> m_turnLight;
	FrameQueue<tagWiper> m_wiper;
	FrameQueue<tagAutoPicture> m_autoPicture;
	FrameQueue<tagAutoQuestion> m_autoQuestion;
	FrameQueue<tagAudio> m_audio;
	FrameQueue<tagFog> m_fog;
	FrameQueue<tagCamera> m_camera;
	std::vector<FrameQueue<tagModel> > m_models;
	std::vector<FrameQueue<tagClickBox> >m_click;
};

int main()
{
	SANClickboxQuestionId a = {12},b = {13};
	printf("%d\r\n",a<b);

	filestructure f;
	f.AddModel("x1","data/model/xyz.obj");
	f.AddModel("x2","data/model/xyz1.obj");
	f.AddModel("x3","data/model/xyz2.obj");
	f.AddModel("x4","data/model/xyz3.obj");


	f.WriteFile("test.wujiecao");
	f.ReadFile("test.wujiecao");
//	;
//	UniqueSource<SANOBJ> m;
//	SANOBJ t = {"123","456"};
//	SANOBJ t2 = {"1233","456"};
//	SANOBJ t3 = {"1232","456"};
//	SANOBJ t4 = {"1231","456"};
//	SANOBJ t5 = {"1230","456"};
//	SANOBJ t6 = {"123","4564"};
//	m.Add(t);
//	m.Add(t2);
//	m.Add(t3);
//	m.Add(t4);
//	m.Add(t5);
//	m.Add(t6);
//	printf("Added\r\n");
//	FILE* fp = nullptr;
//	fp = fopen("test.b","wb");
//	if(fp == nullptr)
//	{
//		printf("Failed...\r\n");
//	}
//
//	for(const auto& _param : m.m_map_source_to_index)
//		printf("%s %d\r\n",_param.first.path,_param.second);
//	for(int i=0;i<m.m_result.size();i++)
//		printf("%d ",m.m_result[i]);
//	printf("\r\n");
//	bool ret = false;
//	ret = m.WriteFile(fp);
//	if(ret)
//	{
//		printf("Writed!\r\n");
//		fclose(fp);
//	}
//	fp = fopen("test.b","rb");
//	if(fp == nullptr)
//	{
//		printf("Failed...\r\n");
//	}
//	ret = m.ReadFile(fp);
//	fclose(fp);
//	printf("Readed\r\n");
//	for(const auto& _param : m.m_map_source_to_index)
//		printf("%s %d\r\n",_param.first.path,_param.second);
//	for(int i=0;i<m.m_result.size();i++)
//		printf("%d ",m.m_result[i]);
//	printf("\r\n");
	return 0;
}
