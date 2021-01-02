/**********************************************************************************************************************
功能：根据给定的树结构, 输出用于展示路径的html文件。
创建时间: 2020.12.20
创建者: 刘海信
***********************************************************************************************************************/

#include <iostream>
#include <string>
#include <fstream>

#include "stdio.h"
#include "unistd.h"

#include "cJSON.h"
#include "tinyxml2.h"

#include "psctree.h"
#include "traceGraph.h"

using namespace tinyxml2;

////////////////////////////////////////////////////
CTraceGraphHtml::CTraceGraphHtml()
{
    memset( m_szDir, 0, sizeof( char ) * PATH_MAX );
    snprintf( m_szFileName, NAME_MAX, "template.html" );

    m_pDataJson  = NULL;
    m_pLinksJson = NULL;
}

/********************************************************************************************
功能：根据指定的目录设置输出目录。
参数：
    输入: pDir—指定的目录名。
    输出: 无。
返回: 无。
说明: 目录名应当为绝对目录。
*********************************************************************************************/
void CTraceGraphHtml::SetOutputDir( char* pDir )
{
    if( pDir == NULL )
        return;

    snprintf( m_szDir, PATH_MAX, "%s", pDir );
}

/********************************************************************************************
功能：根据指定的文件名设置输出文件名。
参数：
    输入: pFileName—指定的文件名。
    输出: 无。
返回: 无。
说明: 无。
*********************************************************************************************/
void CTraceGraphHtml::SetFieName( char* pFileName )
{
    if( pFileName == NULL )
        return;

    snprintf( m_szFileName, NAME_MAX, "%s", pFileName );
}

/********************************************************************************************
功能：根据给定的树初始化。
参数：
    输入: pTree—树结构指针。
    输出: 无。
返回: 初始化成功返回true, 否则返回false。
说明: 无。
*********************************************************************************************/
bool CTraceGraphHtml::SetData( CPscTree* pTree )
{
    if( pTree == NULL )
        return false;

    if( !SetDataJson( pTree ) )
        return false;

    if( !SetLinksJson( pTree ) )
        return false;

    return true;
}

/********************************************************************************************
功能：根据给定的树填充设备数据json数组。
参数：
    输入: pTree—树结构指针。
    输出: 无。
返回: 初始化成功返回true, 否则返回false。
说明: 无。
*********************************************************************************************/
bool CTraceGraphHtml::SetDataJson( CPscTree* pTree )
{
    CTreeNodePtrList rTreeNodeList;

    CTreeNode* pRootNode = pTree->GetRoot();
    pTree->GetAllSubTreeNode( pRootNode, rTreeNodeList );

    m_pDataJson = cJSON_CreateArray();

    rTreeNodeList.GotoHead();
    while( !rTreeNodeList.NextIsNull() )
    {
        TreeNode_PTR   rTreeNode = rTreeNodeList.GetNext();
        CTreeNodeData& rNodeData = rTreeNode->m_data;

        int iEqId = rNodeData.m_no;
        //CTreeDev *pTreeDev = (CTreeDev *)rNodeData.m_ptr;
        //if (pTreeDev == NULL)
        //    continue;

        cJSON* dataObj = cJSON_CreateObject();
        //if (pTreeDev->m_iDevType == OUT_BK)
        {
            cJSON_AddStringToObject( dataObj, "name", GetDevDesc( iEqId ) );
            cJSON_AddNumberToObject( dataObj, "symbolSize", 60 );
            cJSON_AddStringToObject( dataObj, "symbol", "circle" );

            //出口开关专属属性：颜色
            cJSON* tmp1 = cJSON_CreateObject();
            cJSON* tmp2 = cJSON_CreateObject();
            cJSON_AddItemToObject( dataObj, "itemStyle", tmp1 );
            cJSON_AddItemToObject( tmp1, "normal", tmp2 );
            cJSON_AddItemToObject( tmp2, "color", cJSON_CreateString( "green" ) );
        }
        //else
        //{
        //    cJSON_AddStringToObject(dataObj, "name", GetDevDesc(pTreeDev->m_iDevId));
        //    cJSON_AddNumberToObject(dataObj, "symbolSize", 60);
        //    cJSON_AddStringToObject(dataObj, "symbol", "rect");
        //}

        cJSON_AddItemToArray( m_pDataJson, dataObj );
    }

    return true;
}

/********************************************************************************************
功能：根据给定的树填充设备数据json数组。
参数：
    输入: pTree—树结构指针。
    输出: 无。
返回: 初始化成功返回true, 否则返回false。
说明: 无。
*********************************************************************************************/
bool CTraceGraphHtml::SetLinksJson( CPscTree* pTree )
{
    CTreeNodePtrList rTreeNodeList;

    m_pLinksJson = cJSON_CreateArray();

    CTreeNode* pRootNode = pTree->GetRoot();
    FillupLinksJson( pRootNode );

    return true;
}

/********************************************************************************************
功能：填充指定节点的设备数据到json数组。
参数：
    输入: pCurNode—树节点指针。
    输出: 无。
返回: 无。
说明: 无。
*********************************************************************************************/
void CTraceGraphHtml::FillupLinksJson( CTreeNode* pCurNode )
{
    if( pCurNode == NULL )
        return;

    CTreeNode* pChildNode = pCurNode->GetFirstChild();
    while( pChildNode != NULL )
    {
        FillupLinksJson( pChildNode );

        CTreeNodeData& rParentNodeData = pCurNode->m_data;
        CTreeNodeData& rChildNodeData  = pChildNode->m_data;

        FillupOneLinkJson( rParentNodeData, rChildNodeData );

        pChildNode = pChildNode->GetNextNeighbor();
    }
}

/********************************************************************************************
功能：根据父、子节点数据填充links json数组。
参数：
    输入: rParentNodeData—父节点数据; rChildNodeData-子节点数据。
    输出: 无。
返回: 无。
说明: 无。
*********************************************************************************************/
void CTraceGraphHtml::FillupOneLinkJson( CTreeNodeData& rParentNodeData, CTreeNodeData& rChildNodeData )
{
    /*
    CTreeDev *pParentDev = (CTreeDev *)rParentNodeData.m_ptr;
    CTreeDev *pChildDev = (CTreeDev *)rChildNodeData.m_ptr;

    if (pParentDev == NULL || pChildDev == NULL)
        return;

    cJSON *linksObj = cJSON_CreateObject();
    cJSON_AddStringToObject(linksObj, "source", GetDevDesc(pParentDev->m_iDevId));
    cJSON_AddStringToObject(linksObj, "target", GetDevDesc(pChildDev->m_iDevId));
    cJSON_AddItemToArray(m_pLinksJson, linksObj);
*/
    int parentNo = rParentNodeData.m_no;
    int childNo  = rChildNodeData.m_no;
    if( parentNo == 0 || childNo == 0 )
        return;

    cJSON* linksObj = cJSON_CreateObject();
    cJSON_AddStringToObject( linksObj, "source", GetDevDesc( parentNo ) );
    cJSON_AddStringToObject( linksObj, "target", GetDevDesc( childNo ) );
    cJSON_AddItemToArray( m_pLinksJson, linksObj );
}

const char* CTraceGraphHtml::GetDevDesc( int iDevId )
{
    static char s_szDesc[256];

    snprintf( s_szDesc, 256, "SW%d", iDevId );
    return s_szDesc;
}

bool CTraceGraphHtml::OutputFile()
{
    if( strlen( m_szDir ) == 0 )
    {
        printf( "未指定文件输出目录, 默认输出到当前目录\n" );
    }

    if( chdir( m_szDir ) != 0 )
    {
        perror( "m_szDir" );
        return false;
    }
}

/********************************************************************************************
功能：输出html文件。
参数：
    输入: pTree—树结构指针。
    输出: 无。
返回: 初始化成功返回true, 否则返回false。
说明: 文件结构如下所示
    <!DOCTYPE html>
    <html>
        <head>
           <meta charset="UTF-8"/>
           <title>Awesome-pyecharts</title>
           <script type="text/javascript" src="https://assets.pyecharts.org/assets/echarts.min.js"></script>
        </head>
        <body>
            <div id="795053e1395747b8a049d8a2a45b927d" class="chart-container" style="width:1632px; height:918px;"/>
            <script> ... json数据 ... </script>
        </body>
    </html>
*********************************************************************************************/
bool CTraceGraphHtml::OutputHtml()
{
    XMLDocument doc;
    const char* declaration = "<!DOCTYPE html>";
    doc.Parse( declaration );

    //html模块
    XMLElement* root = doc.NewElement( "html" );
    doc.InsertEndChild( root );

    //head模块
    XMLElement* head = doc.NewElement( "head" );
    root->InsertEndChild( head );

    XMLElement* meta = doc.NewElement( "meta" );
    meta->SetAttribute( "charset", "UTF-8" );
    head->InsertEndChild( meta );

    XMLElement* title = doc.NewElement( "title" );
    title->InsertEndChild( doc.NewText( "Awesome-pyecharts" ) );
    head->InsertEndChild( title );

    XMLElement* script = doc.NewElement( "script" );
    script->SetAttribute( "type", "text/javascript" );
    //script->SetAttribute("src", "https://assets.pyecharts.org/assets/echarts.min.js");
    script->SetAttribute( "src", "./echarts.min.js" );
    script->InsertEndChild( doc.NewText( "" ) );
    head->InsertEndChild( script );
    //head模块结束

    //开始处理body模块
    XMLElement* body = doc.NewElement( "body" );
    root->InsertEndChild( body );

    XMLElement* div = doc.NewElement( "div" );
    div->SetAttribute( "id", "795053e1395747b8a049d8a2a45b927d" );
    div->SetAttribute( "class", "chart-container" );
    div->SetAttribute( "style", "width:1632px; height:918px;" );
    body->InsertEndChild( div );

    script = doc.NewElement( "script" );
    std::string strContent;
    strContent += "\nvar chart_795053e1395747b8a049d8a2a45b927d = echarts.init(\n";
    strContent += "document.getElementById('795053e1395747b8a049d8a2a45b927d'), 'white', {renderer: 'canvas'});\n";
    strContent += "var option_795053e1395747b8a049d8a2a45b927d = ";

    //整理script之中的json数据
    char* pJsonInfo = NULL;
    GetJsonInfo( pJsonInfo );

    strContent += pJsonInfo;
    strContent += ";";
    strContent += "chart_795053e1395747b8a049d8a2a45b927d.setOption(option_795053e1395747b8a049d8a2a45b927d);\n";

    script->InsertEndChild( doc.NewText( strContent.c_str() ) );
    body->InsertEndChild( script );
    //body模块结束

    if( pJsonInfo != NULL )
    {
        free( pJsonInfo );
        pJsonInfo = NULL;
    }

    if( strlen( m_szDir ) != 0 )
    {
        if( chdir( m_szDir ) != 0 )
        {
            perror( "m_szDir" );
        }
    }

    if( doc.SaveFile( m_szFileName ) == XML_SUCCESS )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/********************************************************************************************
功能：输出html文件。
参数：
    输入: pJsonInfo—json数据空指针。
    输出: pJsonInfo-已经获取到json数据的指针。
返回: 无。
说明: 获取js需要的json数据。
*********************************************************************************************/
void CTraceGraphHtml::GetJsonInfo( char*& pJsonInfo )
{
    const char* c_color[24] = {
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
        "#918597" };

    cJSON* root = cJSON_CreateObject();

    cJSON_AddBoolToObject( root, "animation", true );
    cJSON_AddNumberToObject( root, "animationThreshold", 2000 );
    cJSON_AddNumberToObject( root, "animationDuration", 1000 );
    cJSON_AddStringToObject( root, "animationEasing", "cubicOut" );
    cJSON_AddNumberToObject( root, "animationDelay", 0 );
    cJSON_AddNumberToObject( root, "animationDurationUpdate", 300 );
    cJSON_AddStringToObject( root, "animationEasingUpdate", "cubicOut" );
    cJSON_AddNumberToObject( root, "animationDelayUpdate", 0 );

    //创建数组的2种形式, color
    //cJSON *array = cJSON_AddArrayToObject(root, "color");
    //cJSON_AddItemToArray(array, cJSON_CreateString("#c23531"));
    cJSON_AddItemToObject( root, "color", cJSON_CreateStringArray( c_color, 24 ) );

    //添加series
    cJSON* series = cJSON_CreateArray();
    cJSON* obj    = cJSON_CreateObject();
    cJSON_AddStringToObject( obj, "type", "graph" );
    cJSON_AddStringToObject( obj, "layout", "force" );
    cJSON_AddNumberToObject( obj, "symbolSize", 10 );

    cJSON* circular = cJSON_CreateObject();
    cJSON_AddBoolToObject( circular, "rotateLabel", false );
    cJSON_AddItemToObject( obj, "circular", circular );

    cJSON* force = cJSON_CreateObject();
    cJSON_AddNumberToObject( force, "repulsion", 8000 );
    cJSON_AddNumberToObject( force, "edgeLength", 50 );
    cJSON_AddNumberToObject( force, "gravity", 0.2 );
    cJSON_AddItemToObject( obj, "force", force );

    cJSON* label = cJSON_CreateObject();
    cJSON_AddBoolToObject( label, "show", true );
    cJSON_AddStringToObject( label, "position", "top" );
    cJSON_AddNumberToObject( label, "margin", 8 );
    cJSON_AddItemToObject( obj, "label", label );

    cJSON* lineStyle = cJSON_CreateObject();
    cJSON_AddBoolToObject( lineStyle, "show", true );
    cJSON_AddNumberToObject( lineStyle, "width", 1 );
    cJSON_AddNumberToObject( lineStyle, "opacity", 1 );
    cJSON_AddNumberToObject( lineStyle, "curveness", 0 );
    cJSON_AddStringToObject( lineStyle, "tyep", "solid" );
    cJSON_AddItemToObject( obj, "lineStyle", lineStyle );

    cJSON_AddBoolToObject( obj, "roam", true );
    cJSON_AddBoolToObject( obj, "draggable", true );
    cJSON_AddBoolToObject( obj, "focusNodeAdjacency", true );

    //添加data数组
    cJSON_AddItemToObject( obj, "data", m_pDataJson /*data*/ );
    //data数组结束

    cJSON* edgeLabel = cJSON_CreateObject();
    cJSON_AddBoolToObject( edgeLabel, "show", false );
    cJSON_AddStringToObject( edgeLabel, "position", "top" );
    cJSON_AddNumberToObject( edgeLabel, "margin", 8 );
    cJSON_AddItemToObject( obj, "edgeLabel", edgeLabel );

    cJSON* edgeSymbol = cJSON_CreateArray();
    cJSON_AddItemToArray( edgeSymbol, cJSON_CreateNull() );
    cJSON_AddItemToArray( edgeSymbol, cJSON_CreateNull() );
    cJSON_AddItemToObject( obj, "edgeSymbol", edgeSymbol );

    cJSON_AddNumberToObject( obj, "edgeSymbolSize", 10 );

    //添加links数组
    cJSON_AddItemToObject( obj, "links", m_pLinksJson );

    //series结束
    cJSON_AddItemToArray( series, obj );
    cJSON_AddItemToObject( root, "series", series );

    //添加legend数组
    cJSON* legend    = cJSON_CreateArray();
    cJSON* legendObj = cJSON_CreateObject();
    cJSON_AddArrayToObject( legendObj, "data" );
    cJSON_AddItemToObject( legendObj, "selected", cJSON_CreateObject() );
    cJSON_AddBoolToObject( legendObj, "show", true );
    cJSON_AddNumberToObject( legendObj, "padding", 5 );
    cJSON_AddNumberToObject( legendObj, "itemGap", 10 );
    cJSON_AddNumberToObject( legendObj, "itemWidth", 25 );
    cJSON_AddNumberToObject( legendObj, "itemHeight", 14 );
    cJSON_AddItemToArray( legend, legendObj );

    cJSON_AddItemToObject( root, "legend", legend );
    //添加legend数组结束

    cJSON* tooltip = cJSON_CreateObject();
    cJSON_AddBoolToObject( tooltip, "show", true );
    cJSON_AddStringToObject( tooltip, "trigger", "item" );
    cJSON_AddStringToObject( tooltip, "triggerOn", "mousemove|click" );
    cJSON* tmp3 = cJSON_CreateObject();
    cJSON_AddItemToObject( tooltip, "axisPointer", tmp3 );
    cJSON_AddStringToObject( tmp3, "type", "line" );
    cJSON_AddBoolToObject( tooltip, "showContent", true );
    cJSON_AddBoolToObject( tooltip, "alwaysShowContent", false );
    cJSON_AddNumberToObject( tooltip, "showDelay", 0 );
    cJSON_AddNumberToObject( tooltip, "hideDelay", 100 );
    cJSON* tmp4 = cJSON_CreateObject();
    cJSON_AddItemToObject( tooltip, "textStyle", tmp4 );
    cJSON_AddNumberToObject( tmp4, "fontSize", 14 );
    cJSON_AddNumberToObject( tooltip, "borderWidth", 0 );
    cJSON_AddNumberToObject( tooltip, "padding", 5 );
    cJSON_AddItemToObject( root, "tooltip", tooltip );

    cJSON* title    = cJSON_CreateArray();
    cJSON* titleObj = cJSON_CreateObject();
    cJSON_AddStringToObject( titleObj, "text", "Test chenzhiqiang" );
    cJSON_AddNumberToObject( titleObj, "padding", 5 );
    cJSON_AddNumberToObject( titleObj, "itemGap", 10 );
    cJSON_AddItemToArray( title, titleObj );
    cJSON_AddItemToObject( root, "title", title );

    char*  pContent = cJSON_Print( root );
    size_t len      = strlen( pContent );
    pJsonInfo       = (char*)malloc( sizeof( char ) * ( len + 1 ) );
    memset( pJsonInfo, 0, sizeof( char ) * ( len + 1 ) );
    memcpy( pJsonInfo, pContent, sizeof( char ) * len );

    printf( "len: %lu\n", strlen( pContent ) );

    //printf("%s\n", pContent);

END:
    cJSON_Delete( root );
    return;
}
