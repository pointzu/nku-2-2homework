#include "algaecell.h"
#include "gamegrid.h"

AlgaeCell::AlgaeCell(int row, int col, GameGrid* parent)
    : QObject(parent), m_row(row), m_col(col), m_grid(parent),
    m_type(AlgaeType::NONE), m_status(NORMAL),
    m_productionMultiplier(1.0), m_timeSinceLightLow(0.0) {

    m_properties = AlgaeType::getProperties(m_type);
}

AlgaeCell::~AlgaeCell() {
}

bool AlgaeCell::plant(AlgaeType::Type type) {
    // Already occupied
    if (isOccupied()) {
        return false;
    }

    // Get light level for this position
    double lightLevel = m_grid->getLightAt(m_row);

    // Get properties for the algae type
    AlgaeType::Properties props = AlgaeType::getProperties(type);

    // Check if there's enough light
    if (lightLevel < props.lightRequiredPlant) {
        return false;
    }

    // Set type and properties
    m_type = type;
    m_properties = props;
    m_status = NORMAL;
    m_productionMultiplier = 1.0;
    m_timeSinceLightLow = 0.0;

    emit cellChanged();

    return true;
}

void AlgaeCell::remove() {
    if (m_type != AlgaeType::NONE) {
        m_type = AlgaeType::NONE;
        m_properties = AlgaeType::getProperties(m_type);
        m_status = NORMAL;
        m_productionMultiplier = 1.0;
        m_timeSinceLightLow = 0.0;

        // Apply removal bonus to surrounding cells
        if (m_grid) {
            m_grid->applyRemoveBonus(m_row, m_col);
        }

        emit cellChanged();
    }
}

void AlgaeCell::update(double deltaTime) {
    if (!isOccupied()) {
        return;
    }

    // Check conditions and update status
    updateStatus();}

    // Apply special rules based on
