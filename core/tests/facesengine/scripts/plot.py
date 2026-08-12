#!/usr/bin/env python3

# ============================================================
#
# This file is a part of digiKam project
# https://www.digikam.org
#
# Date        : 2020-08-11
# Description : A script to plot UMAP projection of the face embedding from Extended Yale B dataset.
#
# SPDX-FileCopyrightText: 2020, Nghia Duong, <minhnghiaduong997 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# ============================================================ */

import numpy as np
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import umap.umap_ as umap
import json

sns.set(style='white', context='notebook', rc={'figure.figsize':(14,10)})

with open('Error_images.json') as f:
    jsonData = json.load(f)

faceEmbeddings = []
labels = []
for json in jsonData:
    faceEmbeddings.append(json['faceembedding'])
    labels.append(json['id'])

reducer = umap.UMAP(n_neighbors=15, metric='euclidean', min_dist=0.1, n_components=2, transform_seed=42)
reducer.fit(faceEmbeddings)

reducedEmbedding = reducer.transform(faceEmbeddings)

plt.scatter(reducedEmbedding[:, 0], reducedEmbedding[:, 1], c=labels, cmap='Spectral', s=5)
plt.gca().set_aspect('equal', 'datalim')
plt.colorbar(boundaries=np.arange(11)-0.5).set_ticks(np.arange(10))
plt.title('UMAP projection of the face embedding from Extended Yale B dataset', fontsize=24);
plt.show()

'''
for i in range(1, 50):
    X_train, X_test, y_train, y_test = train_test_split(faceEmbeddings,
                                                        labels,
                                                        stratify=labels,
                                                        random_state=42)

    reducer = umap.UMAP(n_neighbors=15, metric='euclidean', min_dist=0.1, n_components=i*2, transform_seed=42)
    reducer.fit(X_train, y=y_train)

    reduced_X_train = reducer.transform(X_train)
    reduced_X_test  = reducer.transform(X_test)

    svc = SVC().fit(reduced_X_train, y_train)
    knn = KNeighborsClassifier().fit(reduced_X_train, y_train)

    print(i*2)
    print(svc.score(reduced_X_test, y_test), knn.score(reduced_X_test, y_test))

# split face embedding to train and test sets
X_train, X_test, y_train, y_test = train_test_split(faceEmbeddings,
                                                    labels,
                                                    stratify=labels,
                                                    random_state=42)

reducer = umap.UMAP(n_neighbors=15, metric='euclidean', min_dist=0.1, n_components=28, transform_seed=42)
reducer.fit(X_train, y=y_train)

reduced_X_train = reducer.transform(X_train)
reduced_X_test  = reducer.transform(X_test)

svc = SVC().fit(reduced_X_train, y_train)
knn = KNeighborsClassifier().fit(reduced_X_train, y_train)

print(svc.score(reduced_X_test, y_test), knn.score(reduced_X_test, y_test))

reducedEmbedding = reducer.transform(faceEmbeddings)

plt.scatter(reducedEmbedding[:, 0], reducedEmbedding[:, 1], c=labels, cmap='Spectral', s=5)
plt.gca().set_aspect('equal', 'datalim')
plt.colorbar(boundaries=np.arange(11)-0.5).set_ticks(np.arange(10))
plt.title('UMAP projection of the face embedding', fontsize=24);
plt.show()
'''
