#define _CRT_SECURE_NO_WARNINGS
#include "Tokens.h"

TokensTable::HashNode TokensTable::hashTable[TOKEN_ITEM_SIZE];

int TokensTable::AddTokens(TOKENS_DESCRIBE token)
{
	int nHashCode = GetHashCode(token.szDesc, TOKEN_ITEM_SIZE);
	if (hashTable[nHashCode].token.ntokens == -1)
		hashTable[nHashCode].token = token;
	else
	{
		if (hashTable[nHashCode].token.ntokens == token.ntokens)
			return token.ntokens;
		else
		{
			HashNode*& pNode = hashTable[nHashCode].pNext;
			while (pNode)
			{
				if (pNode->token.ntokens == token.ntokens)
					return token.ntokens;
				pNode = pNode->pNext;
			}
			pNode = new HashNode;
			pNode->token = token;
		}
	}
	return -1;
}

int TokensTable::GetHashCode(const char* pWord, int tablesize)
{
	int hashval = 0;
	const char* szBegin = pWord;
	while (*szBegin)
	{
		hashval = 37 * hashval + *szBegin;
		hashval %= tablesize;
		szBegin++;
	}
	if (hashval < 0)
		hashval += tablesize;
	return hashval;
}

bool TokensTable::FindToken(const char* pWord, int& nToken)
{
	nToken = -1;
	int nHashCode = GetHashCode(pWord, TOKEN_ITEM_SIZE);
	HashNode hashNode = hashTable[nHashCode];
	if (strcmp(hashNode.token.szDesc, pWord) == 0)
	{
		nToken = hashNode.token.ntokens;
		return true;
	}
	else
	{
		HashNode* pNode = hashNode.pNext;
		while (pNode)
		{
			if (strcmp(pNode->token.szDesc, pWord) == 0)
			{
				nToken = pNode->token.ntokens;
				return true;
			}
			pNode = pNode->pNext;
		}
	}
	return false;
}

void AddAllTokens()
{
	for (int i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++)
		TokensTable::AddTokens(tokens[i]);
}