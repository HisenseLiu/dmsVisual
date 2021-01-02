#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "psctree.h"

CPscTree* g_pTree = NULL;
cJSON* g_pDataJson = NULL;
cJSON* g_pLinksJson = NULL;

const char* GetDevDesc(int no)
{
    static char s_szDesc[256];

    snprintf(s_szDesc, 256, "SW%d", no);
    return s_szDesc;
}

void SetDataJson(CPscTree *pTree)
{
    CTreeNodePtrList rTreeNodeList;

    CTreeNode *pRootNode = pTree->GetRoot();
    pTree->GetAllSubTreeNode(pRootNode, rTreeNodeList);

    g_pDataJson = cJSON_CreateArray();

    rTreeNodeList.GotoHead();
    while (!rTreeNodeList.NextIsNull())
    {
        TreeNode_PTR rTreeNode = rTreeNodeList.GetNext();
        CTreeNodeData &rNodeData = rTreeNode->m_data;

        int no = rNodeData.m_no;

        cJSON *dataObj = cJSON_CreateObject();
        {
            cJSON_AddStringToObject(dataObj, "name", GetDevDesc(no));
            cJSON_AddNumberToObject(dataObj, "symbolSize", 60);
            cJSON_AddStringToObject(dataObj, "symbol", "rect");
        }

        cJSON_AddItemToArray(g_pDataJson, dataObj);
    }
}

void FillupOneLinkJson(CTreeNodeData &rParentNodeData, CTreeNodeData &rChildNodeData)
{
    int parentNo = rParentNodeData.m_no;
    int childNo = rChildNodeData.m_no;
    if(parentNo == 0 || childNo == 0)
        return;

    cJSON *linksObj = cJSON_CreateObject();
    cJSON_AddStringToObject(linksObj, "source", GetDevDesc(parentNo));
    cJSON_AddStringToObject(linksObj, "target", GetDevDesc(childNo));
    cJSON_AddItemToArray(g_pLinksJson, linksObj);
}

void FillupLinksJson(CTreeNode *pCurNode)
{
    if (pCurNode == NULL)
        return;

    CTreeNode *pChildNode = pCurNode->GetFirstChild();
    while (pChildNode != NULL)
    {
        FillupLinksJson(pChildNode);

        CTreeNodeData &rParentNodeData = pCurNode->m_data;
        CTreeNodeData &rChildNodeData = pChildNode->m_data;

        FillupOneLinkJson(rParentNodeData, rChildNodeData);

        pChildNode = pChildNode->GetNextNeighbor();
    }
}

bool SetLinksJson(CPscTree *pTree)
{
    CTreeNodePtrList rTreeNodeList;

    g_pLinksJson = cJSON_CreateArray();

    CTreeNode *pRootNode = pTree->GetRoot();
    FillupLinksJson(pRootNode);
}



void GetJsonData()
{
    const char *c_color[24] = {
        "#c23531",
        "#2f4554",
        "#61a0a8",
        "#d48265",
        "#749f83",
        "#ca8622",
        "#bda29a",
        "#6e7074",
        "#546570",
        "#c4ccd3",
        "#f05b72",
        "#ef5b9c",
        "#f47920",
        "#905a3d",
        "#fab27b",
        "#2a5caa",
        "#444693",
        "#726930",
        "#b2d235",
        "#6d8346",
        "#ac6767",
        "#1d953f",
        "#6950a1",
        "#918597"};

    cJSON *root = cJSON_CreateObject();

    cJSON_AddBoolToObject(root, "animation", true);
    cJSON_AddNumberToObject(root, "animationThreshold", 2000);
    cJSON_AddNumberToObject(root, "animationDuration", 1000);
    cJSON_AddStringToObject(root, "animationEasing", "cubicOut");
    cJSON_AddNumberToObject(root, "animationDelay", 0);
    cJSON_AddNumberToObject(root, "animationDurationUpdate", 300);
    cJSON_AddStringToObject(root, "animationEasingUpdate", "cubicOut");
    cJSON_AddNumberToObject(root, "animationDelayUpdate", 0);

    //创建数组的2种形式, color
    //cJSON *array = cJSON_AddArrayToObject(root, "color");
    //cJSON_AddItemToArray(array, cJSON_CreateString("#c23531"));
    cJSON_AddItemToObject(root, "color", cJSON_CreateStringArray(c_color, 24));

    //添加series
    cJSON* series = cJSON_CreateArray();
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "type", "graph");
    cJSON_AddStringToObject(obj, "layout", "force");
    cJSON_AddNumberToObject(obj, "symbolSize", 10);
    
    cJSON* circular = cJSON_CreateObject();
    cJSON_AddBoolToObject(circular, "rotateLabel", false);
    cJSON_AddItemToObject(obj, "circular", circular);

    cJSON* force = cJSON_CreateObject();
    cJSON_AddNumberToObject(force, "repulsion", 8000);
    cJSON_AddNumberToObject(force, "edgeLength", 50);
    cJSON_AddNumberToObject(force, "gravity", 0.2);
    cJSON_AddItemToObject(obj, "force", force);

    cJSON* label = cJSON_CreateObject();
    cJSON_AddBoolToObject(label, "show", true);
    cJSON_AddStringToObject(label, "position", "top");
    cJSON_AddNumberToObject(label, "margin", 8);
    cJSON_AddItemToObject(obj, "label", label);

    cJSON* lineStyle = cJSON_CreateObject();
    cJSON_AddBoolToObject(lineStyle, "show", true);
    cJSON_AddNumberToObject(lineStyle, "width", 1);
    cJSON_AddNumberToObject(lineStyle, "opacity", 1);
    cJSON_AddNumberToObject(lineStyle, "curveness", 0);
    cJSON_AddStringToObject(lineStyle, "tyep", "solid");
    cJSON_AddItemToObject(obj, "lineStyle", lineStyle);

    cJSON_AddBoolToObject(obj, "roam", true);
    cJSON_AddBoolToObject(obj, "draggable", true);
    cJSON_AddBoolToObject(obj, "focusNodeAdjacency", true);
    //添加data数组
    // cJSON* data = cJSON_CreateArray();
    // cJSON* dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "FD1");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 60);
    // cJSON_AddStringToObject(dataObj, "symbol", "circle");
    // cJSON* tmp1 = cJSON_CreateObject();
    // cJSON* tmp2 = cJSON_CreateObject();
    // cJSON_AddItemToObject(dataObj, "itemStyle", tmp1);
    // cJSON_AddItemToObject(tmp1, "normal", tmp2);
    // cJSON_AddItemToObject(tmp2, "color", cJSON_CreateString("green"));
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "BS1");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 40);
    // cJSON_AddStringToObject(dataObj, "symbol", "rect");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "BS2");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 40);
    // cJSON_AddStringToObject(dataObj, "symbol", "roundRect");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "BS3");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 40);
    // cJSON_AddStringToObject(dataObj, "symbol", "triangle");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "SW4");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 30);
    // cJSON_AddStringToObject(dataObj, "symbol", "pin");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "SW3");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 30);
    // cJSON_AddStringToObject(dataObj, "symbol", "pin");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "SW2");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 30);
    // cJSON_AddStringToObject(dataObj, "symbol", "pin");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "SW1");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 30);
    // cJSON_AddStringToObject(dataObj, "symbol", "pin");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "TF1");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 20);
    // cJSON_AddStringToObject(dataObj, "symbol", "rect");
    // cJSON_AddItemToArray(data, dataObj);

    // dataObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(dataObj, "name", "TF2");
    // cJSON_AddNumberToObject(dataObj, "symbolSize", 20);
    // cJSON_AddStringToObject(dataObj, "symbol", "rect");
    // cJSON_AddItemToArray(data, dataObj);

    cJSON_AddItemToObject(obj, "data", g_pDataJson/*data*/);
    //data数组结束

    cJSON* edgeLabel = cJSON_CreateObject();
    cJSON_AddBoolToObject(edgeLabel, "show", false);
    cJSON_AddStringToObject(edgeLabel, "position", "top");
    cJSON_AddNumberToObject(edgeLabel, "margin", 8);
    cJSON_AddItemToObject(obj, "edgeLabel", edgeLabel);

    cJSON* edgeSymbol = cJSON_CreateArray();
    cJSON_AddItemToArray(edgeSymbol, cJSON_CreateNull());
    cJSON_AddItemToArray(edgeSymbol, cJSON_CreateNull());
    cJSON_AddItemToObject(obj, "edgeSymbol", edgeSymbol);

    cJSON_AddNumberToObject(obj, "edgeSymbolSize", 10);

    //links数组开始
    // cJSON* links = cJSON_CreateArray();
    // cJSON* linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "SW1");
    // cJSON_AddStringToObject(linksObj, "target", "BS1");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "FD1");
    // cJSON_AddStringToObject(linksObj, "target", "SW1");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "BS1");
    // cJSON_AddStringToObject(linksObj, "target", "SW2");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "BS1");
    // cJSON_AddStringToObject(linksObj, "target", "SW4");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "BS1");
    // cJSON_AddStringToObject(linksObj, "target", "SW3");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "SW3");
    // cJSON_AddStringToObject(linksObj, "target", "TF1");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "SW4");
    // cJSON_AddStringToObject(linksObj, "target", "TF2");
    // cJSON_AddItemToArray(links, linksObj);

    // linksObj = cJSON_CreateObject();
    // cJSON_AddStringToObject(linksObj, "source", "SW2");
    // cJSON_AddStringToObject(linksObj, "target", "BS2");
    // cJSON_AddItemToArray(links, linksObj);

    cJSON_AddItemToObject(obj, "links", g_pLinksJson /*links*/);
    //links数组添加结束

    //series结束
    cJSON_AddItemToArray(series, obj);
    cJSON_AddItemToObject(root, "series", series);
    
    //添加legend数组
    cJSON* legend = cJSON_CreateArray();
    cJSON* legendObj = cJSON_CreateObject();
    cJSON_AddArrayToObject(legendObj, "data");
    cJSON_AddItemToObject(legendObj, "selected", cJSON_CreateObject());
    cJSON_AddBoolToObject(legendObj, "show", true);
    cJSON_AddNumberToObject(legendObj, "padding", 5);
    cJSON_AddNumberToObject(legendObj, "itemGap", 10);
    cJSON_AddNumberToObject(legendObj, "itemWidth", 25);
    cJSON_AddNumberToObject(legendObj, "itemHeight", 14);
    cJSON_AddItemToArray(legend, legendObj);

    cJSON_AddItemToObject(root, "legend", legend);
    //添加legend数组结束

    cJSON *tooltip = cJSON_CreateObject();
    cJSON_AddBoolToObject(tooltip, "show", true);
    cJSON_AddStringToObject(tooltip, "trigger", "item");
    cJSON_AddStringToObject(tooltip, "triggerOn", "mousemove|click");
    cJSON* tmp3 = cJSON_CreateObject();
    cJSON_AddItemToObject(tooltip, "axisPointer", tmp3);
    cJSON_AddStringToObject(tmp3, "type", "line");
    cJSON_AddBoolToObject(tooltip, "showContent", true);
    cJSON_AddBoolToObject(tooltip, "alwaysShowContent", false);
    cJSON_AddNumberToObject(tooltip, "showDelay", 0);
    cJSON_AddNumberToObject(tooltip, "hideDelay", 100);
    cJSON* tmp4 = cJSON_CreateObject();
    cJSON_AddItemToObject(tooltip, "textStyle", tmp4);
    cJSON_AddNumberToObject(tmp4, "fontSize", 14);
    cJSON_AddNumberToObject(tooltip, "borderWidth", 0);
    cJSON_AddNumberToObject(tooltip, "padding", 5);
    cJSON_AddItemToObject(root, "tooltip", tooltip);

    cJSON* title = cJSON_CreateArray();
    cJSON* titleObj = cJSON_CreateObject();
    cJSON_AddStringToObject(titleObj, "text", "Test chenzhiqiang");
    cJSON_AddNumberToObject(titleObj, "padding", 5);
    cJSON_AddNumberToObject(titleObj, "itemGap", 10);
    cJSON_AddItemToArray(title, titleObj);
    cJSON_AddItemToObject(root, "title", title);

    char *string = cJSON_Print(root);
    printf("len: %lu\n", strlen(string));
    printf("%s\n", string);

END:
    cJSON_Delete(root);
    return;
}


int main()
{
    g_pTree = new CPscTree();

    g_pTree->InsertNode(NULL, CTreeNodeData((void*)NULL));
    CTreeNode* pRoot = g_pTree->GetRoot();
    CTreeNode* pNewNode = g_pTree->InsertNode(pRoot, CTreeNodeData(1));
    pNewNode = g_pTree->InsertNode(pNewNode, CTreeNodeData(2));
    CTreeNode* pNewNode3 = g_pTree->InsertNode(pNewNode, CTreeNodeData(3));
    pNewNode = g_pTree->InsertNode(pNewNode, CTreeNodeData(4));
    pNewNode = g_pTree->InsertNode(pNewNode, CTreeNodeData(5));
    pNewNode = g_pTree->InsertNode(pNewNode3, CTreeNodeData(6));

    SetDataJson(g_pTree);
    SetLinksJson(g_pTree);

    GetJsonData();
    return 1;
}
