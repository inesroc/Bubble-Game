#include <math.h>
#include <algorithm> 
#include <map>
#include <set> 
#include <list>
#include <iterator>

using namespace std;

const int MAX_DISAPPEARED = 50;

// Code based on this:
// https://www.pyimagesearch.com/2018/07/23/simple-object-tracking-with-opencv/

typedef struct {
	float dist;
	int idx;
} Data;

struct compareData {
	bool operator()(const Data d1, const Data d2) {
		if (d1.dist < d2.dist) return true;
		if (d1.dist > d2.dist) return false;
		return false;
	}
};


class CentroidTracker {
public:

	int nextObjectId;
	map<int, pair <int, int>> objects;
	map<int, int> disappeared;

	CentroidTracker() {
		nextObjectId = 0;
	}

	void registerCentroid(pair<int, int> point) {
		objects.insert({ nextObjectId, point });
		disappeared.insert({ nextObjectId, 0 });
		nextObjectId++;
	}

	void deregister(int objectId) {
		objects.erase(objectId);
		disappeared.erase(objectId);
	}

	std::set<int> getKeys() {
		std::set<int> keys;
		std::map<int, pair <int, int>>::iterator itr;
		for (itr = objects.begin(); itr != objects.end(); ++itr) {
			int key = itr->first;
			keys.insert(key);
		}
		return keys;
	}

	// If an object dissapeared for MAX_DISAPPEARED(50) frames we will remove it
	void markObjectsDisappeared() {
		map<int, int>::iterator itr;
		for (itr = disappeared.begin(); itr != disappeared.end(); ++itr) {
			int key = itr->first;
			disappeared[key] ++;
			if (disappeared[key] == MAX_DISAPPEARED) {
				deregister(key);
				itr++;
			}
		}
	}

	// We will only register n balls allowed
	void registerObjects(pair<int, int> centroids[], int sizeCentroid, int numBallsAllowed) {
		for (int i = 0; i < sizeCentroid; i++) {
			registerCentroid(centroids[i]);
			numBallsAllowed--;
			if (numBallsAllowed == 0)
				break;
		}
	}

	map<int, pair <int, int>> update(pair<int, int> centroids[], int sizeCentroid, int numBallsAllowed) {
		// There are no objects in the frame
		// Mark all objects as disapeared
		if (sizeCentroid == 0) {
			markObjectsDisappeared();
			return objects;
		}

		// There are currently no objects we are tracking
		// we’ll register the new objects
		if (objects.size() == 0) {
			registerObjects(centroids, sizeCentroid, numBallsAllowed);
			return objects;
		}

		else {

			list<pair<int, int>> points = EuclideanDistance(centroids, sizeCentroid);
			std::set<int> keys = getKeys();

			// Create a set to save the value of the rows, collumns and centroids
			std::set<int> usedRows;
			std::set<int> usedCols;
			std::set<int> idxCentroids;
			for (int i = 0; i < sizeCentroid; i++) {
				idxCentroids.insert((i));
			}

			std::set<int> usedKey;
			//Get the object IDs 
			int objectsKey[50];
			int i = 0;
			std::map<int, pair <int, int>>::iterator itr;
			for (itr = objects.begin(); itr != objects.end(); ++itr) {
				int key = itr->first;
				objectsKey[i] = key;
				i++;
			}

			// Get the iterators 
			std::list<pair <int, int>>::iterator PointsIt;
			std::list<pair <int, int>>::iterator PointsCentroids;

			// For every point return by the euclidean distance
			for (PointsIt = points.begin(); PointsIt != points.end(); ++PointsIt) {

				// If we already max out our number of ball allowed we stop
				if (numBallsAllowed == 0)
					break;
				else
					numBallsAllowed--;

				int row = PointsIt->first;
				int col = PointsIt->second;
				//If the col or the row was already use -> skip
				if (usedRows.count(row) || usedCols.count(col)) {
					continue;
				}

				// Update object
				// Get Id
				int id = objectsKey[row];
				// Get point
				pair<int, int> pair = centroids[col];
				// update the object
				objects[id] = pair;
				disappeared[id] = 0;

				// can't use this row, col and key anymore
				usedRows.insert(row);
				usedCols.insert(col);
				usedKey.insert(id);
			}

			std::set<int> unusedCols;
			set_difference(idxCentroids.begin(), idxCentroids.end(), usedCols.begin(), usedCols.end(),
				std::inserter(unusedCols, unusedCols.end()));

			/* in the event that the number of object centroids is
			# equal or greater than the number of input centroids
			# we need to check and see if some of these objects have
			# potentially disappeared*/
			if (objects.size() > sizeCentroid) {
				std::map<int, pair <int, int>>::iterator rowIt;
				for (rowIt = objects.begin(); rowIt != objects.end(); ++rowIt) {
					int id = rowIt->first;
					if (!usedRows.count(id)) {
						disappeared[id] += 1;
						if (disappeared[id] > MAX_DISAPPEARED)
							deregister(id);
					}
				}
			}
			/*
			# otherwise, if the number of input centroids is greater
			# than the number of existing object centroids we need to
			# register each new input centroid as a trackable object*/
			else {
				std::set<int>::iterator colIt;
				for (colIt = unusedCols.begin(); colIt != unusedCols.end(); ++colIt) {
					if (numBallsAllowed == 0)
						return objects;
					numBallsAllowed--;
					int idx = itr->first;
					pair <int, int> point = centroids[idx];
					registerCentroid(point);
				}
			}
			return objects;
		}
	}

	/////////////// Auxiliar Functions ///////////////////

	list<pair<int, int>> EuclideanDistance(pair<int, int> centroids[], int sizeCentroid) {

		int row = objects.size();
		int col = sizeCentroid + 1;

		float matrix[50][50]; // num of objects -> num of centroids (new)

		std::map<int, pair <int, int>>::iterator itrOld;
		int i = 0;
		for (itrOld = objects.begin(); itrOld != objects.end(); ++itrOld) {
			int key = itrOld->first;
			int x = itrOld->second.first;
			int y = itrOld->second.second;

			matrix[i][0] = key;
			int j = 1;
			for (int k = 0; k < sizeCentroid; k++) {
				int xNew = centroids[k].first;
				int yNew = centroids[k].second;
				matrix[i][j] = distance(x, y, xNew, yNew);
				j++;
			}
			i++;
		}
		list<pair<int, int>> points = getMin(matrix, row, col);
		return points;
	}

	float distance(int x1, int y1, int x2, int y2) {
		return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
	}

	list<pair<int, int>> getMin(float ar[50][50], int row, int col) {

		Data* dataCol = new Data[col - 1];

		for (int i = 1; i < col; i++) {
			float min = 5000000.0;
			int idx = 0;
			for (int j = 0; j < row; j++) {
				if (ar[j][i] < min) {
					min = ar[j][i];
					idx = i - 1;
				}
			}
			dataCol[i - 1].dist = min;
			dataCol[i - 1].idx = idx;
		}

		sort(dataCol, dataCol + (col - 1), compareData());

		Data* dataRow = new Data[row];
		for (int i = 0; i < row; i++) {
			float min = 5000000.0;
			int idx = 0;
			for (int j = 1; j < col; j++) { // j==0 is the id
				if (ar[i][j] < min) {
					min = ar[i][j];
					idx = i; // ID of objects
				}
			}
			dataRow[i].dist = min;
			dataRow[i].idx = idx;
		}

		sort(dataRow, dataRow + row, compareData());

		// ID of object , id of collumn
		list<pair<int, int>> points;
		if ((col - 1) < row) {
			for (int i = 1; i < col; i++) {
				pair<int, int> pair;
				pair.first = dataRow[i - 1].idx;
				pair.second = dataCol[i - 1].idx;
				points.push_back(pair);
			}
		}
		else {
			for (int i = 0; i < row; i++) {
				pair<int, int> pair;
				pair.first = dataRow[i].idx;
				pair.second = dataCol[i].idx;
				points.push_back(pair);
			}

		}
		std::list<pair <int, int>>::iterator it;
		return points;
	}
};
