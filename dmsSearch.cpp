
extern "C"
{
#include "edbtypes.h"
#include "ecs_meastype.h"
#include "edb_ext.h"
#include "scadadb.h"

#include "pasMsg.h"
#include "pasdbtpar.h"
#include "pasdbtdef.h"
#include "pasdb.h"
#include "pasdbt.h"
}

#include "psctree.h"

#include "dmsSearch.h"

//用于标记深度、广度遍历中各元件的遍历状态
#define TRAVEL_PENDENT 0  //未处理过
#define TRAVEL_VISITED 1  //遍历(访问)过
#define TRAVEL_TOVISIT 2  //已压入待访问堆栈

int CDmsSearch::m_siFilterLeafNum = 0;

CDmsSearch::CDmsSearch()
{
    m_pTeVisitFlagRcd = NULL;
    m_pEqVisitFlagRcd = NULL;
    m_pToVisitstack   = NULL;
    m_pEqSearchLevel  = NULL;

    m_iToVisitNum = 0;
    m_iFeederId   = -1;
}

CDmsSearch::~CDmsSearch()
{
    if( m_pTeVisitFlagRcd != NULL )
    {
        delete[] m_pTeVisitFlagRcd;
        m_pTeVisitFlagRcd = NULL;
    }

    if( m_pEqVisitFlagRcd != NULL )
    {
        delete[] m_pEqVisitFlagRcd;
        m_pEqVisitFlagRcd = NULL;
    }

    if( m_pToVisitstack != NULL )
    {
        delete[] m_pToVisitstack;
        m_pToVisitstack = NULL;
    }

    if( m_pEqSearchLevel != NULL )
    {
        delete[] m_pEqSearchLevel;
        m_pEqSearchLevel = NULL;
    }
}

/********************************************************************************************
功能：判断给定端子是否设备近电源侧端子。
参数：
    输入: iSrcIdx—给定端子索引。
    输出: 无。
返回: 近电源点返回true, 否则返回false。
说明：1.本函数的隐含条件是搜索主网低压侧出口开关或刀闸
     2.本函数通过端子是否可以找到母线，判断是否为近电源侧。
*********************************************************************************************/
#define PAS_SEARCH_TE_MAX_LEVEL 100
bool CDmsSearch::SearchNoSwEqFromTe( int iSrcTeIdx, int& iType, int& iTypeIdx )
{
    int iObjType                             = 0;
    int iAryTeLevel[PAS_SEARCH_TE_MAX_LEVEL] = { 0 };
    int iLevel                               = 0;
    int iFlag                                = 0;
    int iCount                               = 0;
    int iLoopNum                             = 0;

    sint32 iCurTe  = 0;
    sint32 iFromTe = 0;
    sint32 iToTe   = 0;
    sint32 iNd     = 0;
    sint32 iBsIdx  = -1;
    sint32 iSwIdx  = 0;
    sint32 iRbIdx  = 0;

    if( PAS_IS_INVALID( Te, iSrcTeIdx ) )
        return false;

    iNd = PAS_TeSta[iSrcTeIdx].nd;
    if( PAS_IS_INVALID( Nd, iNd ) )
        return false;

    int* pTeFlag = new int[PAS_TeUsedCat->extent + 1];
    int* pNdFlag = new int[PAS_NdUsedCat->extent + 1];
    memset( pTeFlag, 0, sizeof( int ) * ( PAS_TeUsedCat->extent + 1 ) );
    memset( pNdFlag, 0, sizeof( int ) * ( PAS_NdUsedCat->extent + 1 ) );
    memset( iAryTeLevel, 0, sizeof( int ) * PAS_SEARCH_TE_MAX_LEVEL );

    pTeFlag[iSrcTeIdx] = 1;
    iLevel             = 0;
    iCount             = 0;
    while( 1 )
    {
        if( pNdFlag[iNd] == 0 )  //未访问过的nd入栈
        {
            if( iLevel >= PAS_SEARCH_TE_MAX_LEVEL - 1 )
                break;

            iLevel++;
            iAryTeLevel[iLevel] = iNd;
            pNdFlag[iNd]        = 1;
        }

        iCount++;
        if( iCount > 1000 )
        {
            printf( "SearchFromTe COUNT OVER: cur_count = %d, max_count = %d\n", iCount, 1000 );
            break;
        }

        iLoopNum = 0;
        for( iCurTe = PAS_NdSta[iNd].firstTe; PAS_IS_VALID( Te, iCurTe ); iCurTe = PAS_TeSta[iCurTe].nextTe )  //找出当前节点的非开关设备进行判断
        {
            iLoopNum++;
            if( iLoopNum > 999 )
                break;

            iObjType = PAS_TeSta[iCurTe].objType;
            if( iObjType != PAS_TYP_SW )
            {
                if( iObjType == PAS_TYP_RB )  //对电抗器处理
                {
                    iRbIdx = PAS_TeSta[iCurTe].obj;
                    if( PAS_IS_VALID( Rb, iRbIdx ) && PAS_RbSta[iRbIdx].type == PAS_EQ_CON_TYPE_SER )  //串联电抗不做处理
                        continue;
                    else
                        break;
                }
                else
                {
                    if( iCurTe == iSrcTeIdx || pTeFlag[iCurTe] == 1 )
                        continue;
                    else
                        break;
                }
            }
        }

        if( iObjType == PAS_TYP_BS && PAS_IS_VALID( Te, iCurTe ) )  //遇到母线，说明是近电源侧
        {
            iType    = PAS_TYP_BS;
            iTypeIdx = PAS_TeSta[iCurTe].obj;
            return true;
        }
        else if( ( iObjType == PAS_TYP_LN && PAS_IS_VALID( Te, iCurTe ) ) )  //遇到线路，说明是远电源侧
        {
            iType    = PAS_TYP_LN;
            iTypeIdx = PAS_TeSta[iCurTe].obj;
            return true;
        }

        iFlag    = 0;
        iLoopNum = 0;
        for( iCurTe = PAS_NdSta[iNd].firstTe; PAS_IS_VALID( Te, iCurTe ); iCurTe = PAS_TeSta[iCurTe].nextTe )  //找到本Nd还没搜过的Te
        {
            iLoopNum++;
            if( iLoopNum > 999 )
                break;

            if( pTeFlag[iCurTe] == 0 )
            {
                pTeFlag[iCurTe] = 1;
                iFlag++;
                break;
            }
        }

        if( iFlag == 0 )  //本Nd的Te都被搜过了，退栈返回上一层继续搜索
        {
            iLevel--;
            if( iLevel <= 0 )
                break;

            iNd = iAryTeLevel[iLevel];
            continue;
        }

        if( PAS_IS_VALID( Te, iCurTe ) )  //从当前iCurTe通过开关或串联电抗找出下一层要搜索的nd
        {
            if( PAS_TeSta[iCurTe].objType == PAS_TYP_SW )
            {
                iSwIdx = PAS_TeSta[iCurTe].obj;
                if( PAS_IS_VALID( Sw, iSwIdx ) )
                {
                    iFromTe = PAS_SwSta[iSwIdx].fromTe;
                    iToTe   = PAS_SwSta[iSwIdx].toTe;
                    if( iFromTe == iCurTe )
                    {
                        iNd            = PAS_TeSta[iToTe].nd;
                        pTeFlag[iToTe] = 1;
                    }
                    else if( iToTe == iCurTe )
                    {
                        iNd              = PAS_TeSta[iFromTe].nd;
                        pTeFlag[iFromTe] = 1;
                    }
                }
            }
            else if( PAS_TeSta[iCurTe].objType == PAS_TYP_RB )
            {
                iRbIdx = PAS_TeSta[iCurTe].obj;
                if( PAS_IS_VALID( Rb, iRbIdx ) && PAS_RbSta[iRbIdx].type == PAS_EQ_CON_TYPE_SER )
                {
                    iFromTe = PAS_RbSta[iRbIdx].fromTe;
                    iToTe   = PAS_RbSta[iRbIdx].toTe;
                    if( iFromTe == iCurTe )
                    {
                        iNd            = PAS_TeSta[iToTe].nd;
                        pTeFlag[iToTe] = 1;
                    }
                    else if( iToTe == iCurTe )
                    {
                        iNd              = PAS_TeSta[iFromTe].nd;
                        pTeFlag[iFromTe] = 1;
                    }
                }
            }
        }

        if( PAS_IS_INVALID( Nd, iNd ) )  //如果下一层没有nd，则取出本层nd，继续搜索本层未访问过的te
        {
            iNd = iAryTeLevel[iLevel];
        }
    }

    if( pTeFlag )
    {
        delete[] pTeFlag;
        pTeFlag = NULL;
    }

    if( pNdFlag )
    {
        delete[] pNdFlag;
        pNdFlag = NULL;
    }

    return false;
}

/********************************************************************************************
功能：根据线路索引确定所属馈线ID。
参数：
    输入: iType—给定设备类型; iTypeIdx-给定设备索引。
    输出: 无。
返回：确定馈线成功返回true, 否则返回false。
说明：无。
*********************************************************************************************/
bool CDmsSearch::DetermineFeederIdByLnIdx( int iType, int iTypeIdx )
{
    if( iType != PAS_TYP_LN )
        return false;

    if( PAS_IS_INVALID( Ln, iTypeIdx ) )
        return false;

    int iEqIdx = PAS_LnSta[iTypeIdx].eq;
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    int iEqId = PAS_EqSta[iEqIdx].ID;
    if( iEqId <= 0 || iEqId > SCADA_DevidToIdxCat->extent )
        return false;

    uint32 uiDevIdx = SCADA_DevidToIdx[iEqId];
    if( uiDevIdx <= 0 || uiDevIdx > SCADA_DeviceCat->extent )
        return false;

    uint32 uiSubFeederIdx = SCADA_Device[uiDevIdx].subfeeder_idx;
    if( uiSubFeederIdx > 0 && uiSubFeederIdx <= SCADA_SubFeederCat->extent )
    {
        uint32 uiFdIdx = SCADA_SubFeeder[uiSubFeederIdx].feeder_idx;
        if( uiFdIdx > 0 && uiFdIdx <= SCADA_FeederCat->extent )
        {
            m_iFeederId = SCADA_Feeder[uiFdIdx].feederId;
            return true;
        }
    }

    return false;
}

/********************************************************************************************
功能：根据给定设备确定起始搜索端子。
参数：
    输入: iEqId—给定设备ID。
    输出: iSearchTeIdx-起始搜索端子。
返回: 确定成功返回ture, 否则返回false。
说明: 无。
*********************************************************************************************/
bool CDmsSearch::DetermineSearchTe( int iEqId, int& iSearchTeIdx )
{
    iSearchTeIdx = -1;

    if( iEqId <= 0 || iEqId > PAS_EqIDCat->extent )
        return -1;

    int iEqIdx = PAS_EqID[iEqId];
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    int iType    = PAS_EqSta[iEqIdx].type;
    int iTypeIdx = PAS_EqSta[iEqIdx].typeIdx;
    if( iType != PAS_TYP_SW )
    {
        printf( "Error: 只从开关刀闸开始搜索!!!\n" );
        return false;
    }

    if( PAS_IS_INVALID( Sw, iTypeIdx ) )
        return false;

    int iFromTeIdx = PAS_SwSta[iTypeIdx].fromTe;
    int iToTeIdx   = PAS_SwSta[iTypeIdx].toTe;
    int iTargetType, iTargetTypeIdx;
    int iDmsSizeTeIdx = -1;

    SearchNoSwEqFromTe( iFromTeIdx, iTargetType, iTargetTypeIdx );
    if( iTargetType == PAS_TYP_LN )
    {
        iDmsSizeTeIdx = iFromTeIdx;
    }
    else
    {
        SearchNoSwEqFromTe( iToTeIdx, iTargetType, iTargetTypeIdx );
        if( iTargetType == PAS_TYP_LN )
        {
            iDmsSizeTeIdx = iToTeIdx;
        }
    }

    if( PAS_IS_INVALID( Te, iDmsSizeTeIdx ) )
    {
        printf( "Error: 要搜索设备(id:%d)找不到配网设备，无法进行搜索\n" );
        return false;
    }

    iSearchTeIdx = iDmsSizeTeIdx;
    DetermineFeederIdByLnIdx( iTargetType, iTargetTypeIdx );

    return true;
}

/********************************************************************************************
功能：根据给定设备搜索下游自动化开关结尾的树结构。
参数：
    输入: iEqId—给定设备ID。
    输出: pDmsTree-开关、刀闸组成的树结构。
返回: 无。
说明: 无。
*********************************************************************************************/
void CDmsSearch::SearchAutoBkEndTree( int iEqId, CPscTree* pDmsTree )
{
    SearchDmsTree( iEqId, pDmsTree );

    FilterTreeNoAutoBkLeaf( *pDmsTree );
}

/********************************************************************************************
功能：根据给定设备搜索下游开关刀闸设备，以树结构给出。
参数：
    输入: iEqId—给定设备ID。
    输出: pDmsTree-开关、刀闸组成的树结构。
返回: 无。
说明: 无。
*********************************************************************************************/
void CDmsSearch::SearchDmsTree( int iEqId, CPscTree* pDmsTree )
{
    int iSearchTeIdx = -1;
    if( !DetermineSearchTe( iEqId, iSearchTeIdx ) )
    {
        printf( "Error: 无法确定要搜索设备(id:%d)的配网侧端子!!!\n" );
        return;
    }

    int iMaxTeSize = PAS_TeUsedCat->extent + 1;

    if( m_pToVisitstack != NULL )
    {
        delete[] m_pToVisitstack;
        m_pToVisitstack = NULL;
    }

    m_pToVisitstack = new int[iMaxTeSize];
    if( m_pToVisitstack == NULL )
        return;

    if( m_pTeVisitFlagRcd != NULL )
    {
        delete[] m_pTeVisitFlagRcd;
        m_pTeVisitFlagRcd = NULL;
    }

    m_pTeVisitFlagRcd = new int[iMaxTeSize];
    if( m_pTeVisitFlagRcd == NULL )
        return;
    memset( m_pTeVisitFlagRcd, 0, sizeof( int ) * iMaxTeSize );

    if( m_pEqVisitFlagRcd != NULL )
    {
        delete[] m_pEqVisitFlagRcd;
        m_pEqVisitFlagRcd = NULL;
    }

    int iMaxEqSize    = PAS_EqUsedCat->extent + 1;
    m_pEqVisitFlagRcd = new int[iMaxEqSize];
    if( m_pEqVisitFlagRcd == NULL )
        return;
    memset( m_pEqVisitFlagRcd, 0, sizeof( int ) * iMaxEqSize );

    if( m_pEqSearchLevel != NULL )
    {
        delete[] m_pEqSearchLevel;
        m_pEqSearchLevel = NULL;
    }

    m_pEqSearchLevel = new int[iMaxEqSize];
    if( m_pEqSearchLevel == NULL )
        return;
    memset( m_pEqSearchLevel, 0, sizeof( int ) * iMaxEqSize );

    pDmsTree->SetNodeJudgeFunc( NULL );

    int iObj   = PAS_TeSta[iSearchTeIdx].obj;
    int iEqIdx = PAS_SwSta[iObj].eq;
    if( pDmsTree->InsertNode( NULL, CTreeNodeData( PAS_EqSta[iEqIdx].ID ) ) == NULL )
        return;

    BFS_GetTeDownStreamTree( iSearchTeIdx, pDmsTree );
}

/********************************************************************************************
功能：广度搜索得到指定端子下游的开关刀闸形成的树。
参数：
    输入：iSrcTeIdx—起始端子位置;
    输出：pDmsTree—经过广度搜索，得到的树数据结构，树节点按层级保存。
返回：出错返回false；成功返回true。
说明：
    1、采用广度搜索形成层次树；不考虑iSrcTeIdx下游有环网的情况（有环网的话，通过树数据结构不能表达，要改用(子)图）；
    2、树的根节点为空，根节点的子节点保存的才是有效边（因为一个树节点只能对应一个边，iSrcTeIdx可能有多个直接下游边）。
    3、不考虑开关状态。
*********************************************************************************************/
bool CDmsSearch::BFS_GetTeDownStreamTree( int iSrcTeIdx, CPscTree* pDmsTree )
{
    TreeNode_PTR* pParentStack = new TreeNode_PTR[PAS_TeUsedCat->extent + 1];  //记录各个端子的父树节点
    if( pParentStack == NULL )
        return false;
    memset( pParentStack, 0, sizeof( TreeNode_PTR ) * ( PAS_TeUsedCat->extent + 1 ) );

    int step      = 0;
    m_iToVisitNum = 0;

    m_pToVisitstack[m_iToVisitNum] = iSrcTeIdx;
    pParentStack[iSrcTeIdx]        = pDmsTree->GetRoot();
    m_iToVisitNum++;

    //设置起始端子访问标志
    int iFromTeIdx                = m_pToVisitstack[0];
    m_pTeVisitFlagRcd[iFromTeIdx] = TRAVEL_VISITED;

    step++;

    // int iNdIdx = PAS_TeSta[iFromTeIdx].nd;
    // if (PAS_IS_INVALID(Nd, iNdIdx))
    //     return false;

    bool       ret            = true;
    CTreeNode* pCurParentNode = NULL;
    for( int i = 0; i < m_iToVisitNum; i++ )
    {
        int iTeIdx     = m_pToVisitstack[i];
        pCurParentNode = pParentStack[iTeIdx];

        int iNdIdx = PAS_TeSta[iTeIdx].nd;
        if( PAS_IS_INVALID( Nd, iNdIdx ) )
            continue;

        int iCurTeEqIdx = GetEqIdxFromTe( iTeIdx );
        int iLevel      = m_pEqSearchLevel[iCurTeEqIdx];

        int iCurTeIdx = -1, iLoopNum = 0;
        for( iCurTeIdx = PAS_NdSta[iNdIdx].firstTe; PAS_IS_VALID( Te, iCurTeIdx ); iCurTeIdx = PAS_TeSta[iCurTeIdx].nextTe )
        {
            iLoopNum++;
            if( iLoopNum > 99 )
                break;

            if( m_pTeVisitFlagRcd[iCurTeIdx] == TRAVEL_VISITED )
                continue;

            int iEqIdx, iTe1, iTe2, iTe3;
            int sts = GetEqTeFromTe( iCurTeIdx, iEqIdx, iTe1, iTe2, iTe3 );
            if( sts <= 0 )
                continue;

            if( iCurTeIdx == iTe1 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else if( iCurTeIdx == iTe2 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else if( iCurTeIdx == iTe3 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else
            {
                continue;
            }
        }
    }
}

/********************************************************************************************
功能：对广度搜索中遇到的设备入栈，并记录遇到的开关、刀闸设备。
参数：
    输入: iTeIdx—端子索引, iEqIdx-设备索引, pParentNode-入栈端子的树中的父节点, iLevel-入栈设备在广度搜索中的层级。
    输出: pDmsTree-新加入开关、刀闸节点的树结构, pParentStack-更新了端子所属父节点的父节点数组。
返回: 无。
说明: 将新端子存入待访问堆栈，并将开关、刀闸设备插入遍历树，并更新端子所属父节点。
*********************************************************************************************/
void CDmsSearch::InStack( int iTeIdx, int iEqIdx, CPscTree* pDmsTree, CTreeNode*& pParentNode, TreeNode_PTR* pParentStack, int iLevel )
{
    if( pDmsTree == NULL || pParentStack == NULL )
        return;

    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return;

    if( m_pTeVisitFlagRcd[iTeIdx] == TRAVEL_VISITED )
        return;

    CTreeNode* pNewAddNode = pParentNode;
    if( m_pEqSearchLevel[iEqIdx] == 0 )
    {
        m_pEqSearchLevel[iEqIdx] = iLevel;
    }

    if( PAS_EqSta[iEqIdx].type == PAS_TYP_BS )
    {
        if( IsSelfFeedBk( iEqIdx ) )
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }
    else if( PAS_EqSta[iEqIdx].type == PAS_TYP_LN )
    {
        if( IsSelfFeedBk( iEqIdx ) )
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }
    else if( PAS_EqSta[iEqIdx].type == PAS_TYP_SW )
    {
        if( m_pEqVisitFlagRcd[iEqIdx] != TRAVEL_VISITED )  //已经记录过
        {
            pNewAddNode = pDmsTree->InsertNode( pParentNode, CTreeNodeData( PAS_EqSta[iEqIdx].ID ) );
            if( pNewAddNode == NULL )
            {
                abort();
            }

            printf( "level:%d ID:%d 父设备ID:%d\n", iLevel, PAS_EqSta[iEqIdx].ID, pParentNode->m_data.m_no );
        }

        if( IsAutoBk( iEqIdx ) )  //自动化开关需要记录
        {
            if( IsSelfFeedBk( iEqIdx ) )  //自动化开关且本馈线内的入栈
            {
                m_pToVisitstack[m_iToVisitNum] = iTeIdx;
                m_iToVisitNum++;
            }
        }
        else  //非自动化开关都入栈
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }

    pParentStack[iTeIdx]      = pNewAddNode;
    m_pTeVisitFlagRcd[iTeIdx] = TRAVEL_VISITED;
    m_pEqVisitFlagRcd[iEqIdx] = TRAVEL_VISITED;

    pParentNode = pNewAddNode;
}

/********************************************************************************************
功能：判断设备是否属于要搜索的本馈线。
参数：
    输入: iEqIdx—设备索引。
    输出: 无。
返回: 是本馈线内的返回true, 不是返回false。
说明: 无。
*********************************************************************************************/
bool CDmsSearch::IsSelfFeedBk( int iEqIdx )
{
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    uint32 devId = PAS_EqSta[iEqIdx].ID;
    if( devId <= 0 || devId > SCADA_DevidToIdxCat->extent )
        return false;

    uint32 uiDevIdx = SCADA_DevidToIdx[devId];
    if( uiDevIdx <= 0 || uiDevIdx > SCADA_DeviceCat->extent )
        return false;

    uint32 uiSubFeederIdx = SCADA_Device[uiDevIdx].subfeeder_idx;
    if( uiSubFeederIdx > 0 && uiSubFeederIdx <= SCADA_SubFeederCat->extent )
    {
        uint32 uiFdIdx = SCADA_SubFeeder[uiSubFeederIdx].feeder_idx;
        if( uiFdIdx > 0 && uiFdIdx <= SCADA_FeederCat->extent )
        {
            if( SCADA_Feeder[uiFdIdx].feederId == m_iFeederId )
                return true;
            else
                return false;
        }
    }
    else  //无属性的是主网开关
    {
        return true;
    }
}

/********************************************************************************************
功能：判断设备是否自动化开关。
参数：
    输入: iEqIdx—设备索引。
    输出: 无。
返回: 是自动化开关的返回true, 不是返回false。
说明: 无。
*********************************************************************************************/
bool CDmsSearch::IsAutoBk( int iEqIdx )
{
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    if( PAS_EqSta[iEqIdx].type != PAS_TYP_SW )
        return false;

    int iSwIdx = PAS_EqSta[iEqIdx].typeIdx;
    if( PAS_SwSta[iSwIdx].type >= PAS_SW_TYPE_PTONG )
        return false;

    int iEqId = PAS_EqSta[iEqIdx].ID;
    if( iEqId <= 0 || iEqId > SCADA_DevidToIdxCat->extent )
        return false;

    int iDevIdx = SCADA_DevidToIdx[iEqId];
    int iPtIdx  = SCADA_Device[iDevIdx].point_idx;
    if( SCADA_PointType[iPtIdx].point_class == ECS_K_PT_CLASS_AI ||
        SCADA_PointType[iPtIdx].point_class == ECS_K_PT_CLASS_DI )
        return true;
    else
        return false;
}

/********************************************************************************************
功能：获取指定端子的设备索引以及设备的全部端子。
参数：
    输入: iTeIdx—指定端子索引。
    输出: iEqIdx-设备索引, iTe1-端子1, iTe2-端子2, iTe3-端子3。
返回: 获取到设备的端子数量。
说明: 无。
*********************************************************************************************/
int CDmsSearch::GetEqTeFromTe( int iTeIdx, int& iEqIdx, int& iTe1, int& iTe2, int& iTe3 )
{
    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return 0;

    iEqIdx = -1;
    iTe1   = -1;
    iTe2   = -1;
    iTe3   = -1;

    int iObjType = PAS_TeSta[iTeIdx].objType;
    int iObj     = PAS_TeSta[iTeIdx].obj;

    switch( iObjType )
    {
        case PAS_TYP_LN:
            if( PAS_IS_INVALID( Ln, iObj ) )
                break;

            iEqIdx = PAS_LnSta[iObj].eq;
            iTe1   = PAS_LnSta[iObj].fromTe;
            iTe2   = PAS_LnSta[iObj].toTe;
            break;
        case PAS_TYP_SW:
            if( PAS_IS_INVALID( Sw, iObj ) )
                break;

            iEqIdx = PAS_SwSta[iObj].eq;
            iTe1   = PAS_SwSta[iObj].fromTe;
            iTe2   = PAS_SwSta[iObj].toTe;
            break;
        case PAS_TYP_BS:
            if( PAS_IS_INVALID( Bs, iObj ) )
                break;

            iEqIdx = PAS_BsSta[iObj].eq;
            iTe1   = PAS_BsSta[iEqIdx].te;
            break;
        default:
            break;
    }

    int teNum = 0;
    if( PAS_IS_VALID( Te, iTe1 ) )
        teNum++;

    if( PAS_IS_VALID( Te, iTe2 ) )
        teNum++;

    if( PAS_IS_VALID( Te, iTe3 ) )
        teNum++;

    return teNum;
}

/********************************************************************************************
功能：获取端子的设备索引。
参数：
    输入: iTeIdx—端子索引。
    输出: 无。
返回: 端子所属的设备索引。
说明: 无。
*********************************************************************************************/
int CDmsSearch::GetEqIdxFromTe( int iTeIdx )
{
    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return 0;

    int iEqIdx = -1;

    int iObjType = PAS_TeSta[iTeIdx].objType;
    int iObj     = PAS_TeSta[iTeIdx].obj;

    switch( iObjType )
    {
        case PAS_TYP_LN:
            if( PAS_IS_INVALID( Ln, iObj ) )
                break;

            iEqIdx = PAS_LnSta[iObj].eq;
            break;
        case PAS_TYP_SW:
            if( PAS_IS_INVALID( Sw, iObj ) )
                break;

            iEqIdx = PAS_SwSta[iObj].eq;
            break;
        case PAS_TYP_BS:
            if( PAS_IS_INVALID( Bs, iObj ) )
                break;

            iEqIdx = PAS_BsSta[iObj].eq;
            break;
        default:
            break;
    }

    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return 0;

    return iEqIdx;
}

/********************************************************************************************
功能：处理树结构叶子节点的回调函数。
参数：
    输入: treeNodeData—节点数据, pUserObjData-用户数据。
    输出: 无。
返回: 无。
说明: 判断叶子节点保存的是否自动化开关，不是则删除。
*********************************************************************************************/
void CDmsSearch::DisposeDmsTreeNode( CTreeNodeData& treeNodeData, void* pUserObjData )
{
    if( pUserObjData == NULL )
        return;

    CDisposeTreeUserData* pDisposeData = (CDisposeTreeUserData*)pUserObjData;

    CPscTree*   pDmsTree   = pDisposeData->m_pDmsTree;
    CDmsSearch* pDmsSearch = pDisposeData->m_pDmsSearch;

    int iEqId  = treeNodeData.m_no;
    int iEqIdx = PAS_EqID[iEqId];

    if( !pDmsSearch->IsAutoBk( iEqIdx ) )
    {
        //printf( "删除非采集开关叶子节点: %d\n", iEqId );
        pDmsTree->DeleteNode( NULL, treeNodeData );
        m_siFilterLeafNum++;
    }
}

/********************************************************************************************
功能：处理树结构中的非自动化开关叶子节点。
参数：
    输入: rDmsTree—要处理的树结构。
    输出: 无。
返回: 无。
说明: 对树结构中的叶子节点进行循环遍历，每次遍历时把不是自动化开关的叶子节点全部删除，直到所有的叶子节点都是自动化开关。
*********************************************************************************************/
void CDmsSearch::FilterTreeNoAutoBkLeaf( CPscTree& rDmsTree )
{
    CDisposeTreeUserData rDisposeData( &rDmsTree, this );

    rDmsTree.RegisterUserData( (void*)( &rDisposeData ) );
    rDmsTree.PostOrderTraverseLeaf( DisposeDmsTreeNode );

    while( m_siFilterLeafNum > 0 )
    {
        m_siFilterLeafNum = 0;
        rDmsTree.PostOrderTraverseLeaf( DisposeDmsTreeNode );
    }

    rDmsTree.UnRegisterUserData();
}

/********************************************************************************************
功能：输出指定树的模型。
参数：
    输入: rDmsTree—要处理的树结构。
    输出: 无。
返回: 无。
说明: 无。
*********************************************************************************************/
void CDmsSearch::OutTreeModel( CPscTree& rDmsTree )
{
    printf( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    printf( "<Feeder Id=\"%d\">", m_iFeederId );
    printf( "    <DeviceInfo>\n" );

    DFS_TravelOutTree( rDmsTree, rDmsTree.GetRoot() );

    printf( "    </DeviceInfo>\n" );
    printf( "</Feeder>\n" );
}

/********************************************************************************************
功能：深度递归输出树的模型结构。
参数：
    输入: rDmsTree—要处理的树结构, pCurNode-当前处理的树节点。
    输出: 无。
返回: 无。
说明: 无。
*********************************************************************************************/
void CDmsSearch::DFS_TravelOutTree( CPscTree& rDmsTree, CTreeNode* pCurNode )
{
    printf( "        <Device Id=\"%d\"", GetEqIdOfTreeNode( pCurNode ) );
    // int nCurNodeChildNum = rDmsTree.GetChildCount( pCurNode );
    // if( nCurNodeChildNum > 0 )
    // {
    //     //printf( "LinkNum=%d ", nCurNodeChildNum );
    //     //printf( " LinkInfo=\"" );
    // }

    CTreeNode* pChildNode = pCurNode->GetFirstChild();

    int iEqId = 0;
    while( pChildNode != NULL )
    {
        if( iEqId > 0 )
        {
            printf( ";" );
        }
        else
        {
            printf( " LinkInfo=\"" );
        }

        iEqId = GetEqIdOfTreeNode( pChildNode );
        printf( "%d", iEqId );

        pChildNode = pChildNode->GetNextNeighbor();
    }  //while

    if( iEqId > 0 )
        printf( "\"/>\n" );
    else
        printf( "/>\n" );

    pChildNode = pCurNode->GetFirstChild();
    while( pChildNode != NULL )
    {
        DFS_TravelOutTree( rDmsTree, pChildNode );

        pChildNode = pChildNode->GetNextNeighbor();
    }  //while
}

/********************************************************************************************
功能：从树节点获取设备ID。
参数：
    输入: pCurNode-当前处理的树节点。
    输出: 无。
返回: 成功返回设备ID，否则返回-1。
说明: 无。
*********************************************************************************************/
int CDmsSearch::GetEqIdOfTreeNode( CTreeNode* pCurNode )
{
    if( pCurNode == NULL )
    {
        return -1;
    }

    CTreeNodeData& rNodeData = pCurNode->m_data;
    return rNodeData.m_no;
}
