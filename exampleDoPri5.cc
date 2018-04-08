#include <G4DormandPrince745.hh>
#include <VDormandPrince745.hh>
#include <G4UniformMagField.hh>
#include <G4Mag_UsualEqRhs.hh>
#include <G4FieldTrack.hh>
#include <G4Proton.hh>
#include <G4DynamicParticle.hh>
#include <G4FieldUtils.hh>

#include <VecCore/Timer.h>

#include <memory>
#include <iostream>

const G4int INTEGRATED_COMPONENTS = 6;
const G4int NUMBER_OF_INTEGRATION_STEPS = 10000;
using State = G4double[G4FieldTrack::ncompSVEC];

template <typename Stepper, typename Equation>
void test(
    Stepper& method, 
    const Equation& equation, 
    const State& state,
    G4double stepLength)
{
    std::cout << state[3] << " " << state[4] << " " << state[5] << std::endl;
    Timer<microseconds> timer;

    State y, dydx, error;
    memcpy(y, state, sizeof(G4double) * G4FieldTrack::ncompSVEC);

    timer.Start();
    for (G4int i = 0; i < NUMBER_OF_INTEGRATION_STEPS; ++i) {
        equation->RightHandSide(y, dydx);
        method.Stepper(y, dydx, stepLength, y, error);
    }
    G4double time = timer.Elapsed();


    printf("------------------------------------------\n");
    printf(">>  Mean / Sigma (ms)\n");
    printf(">>  %f %f\n", time, 0.);
    printf("------------------------------------------\n");
}

int main()
{
    G4DynamicParticle dynParticle(
            G4Proton::Definition(),
            G4ThreeVector(1, 0, 1).unit(),
            1 * CLHEP::MeV);

    auto track =
        std::make_shared<G4FieldTrack>(
            G4ThreeVector{0., 0., 0.}, // start position
            0,                         // LaboratoryTimeOfFlight
            dynParticle.GetMomentumDirection(),
            dynParticle.GetKineticEnergy(),
            dynParticle.GetMass(),
            dynParticle.GetCharge(),
            dynParticle.GetPolarization());

    auto field = std::make_shared<G4UniformMagField>(
            G4ThreeVector(0, 0, 1 * CLHEP::tesla));

    auto equation = std::make_shared<G4Mag_UsualEqRhs>(field.get());

    equation->SetChargeMomentumMass(
            {
                dynParticle.GetCharge(),
                dynParticle.GetSpin(),
                dynParticle.GetMagneticMoment()
            },
            dynParticle.GetMomentum().mag(),
            dynParticle.GetMass());

    G4DormandPrince745 method(equation.get(), INTEGRATED_COMPONENTS);
    VDormandPrince745 vmethod(equation.get(), INTEGRATED_COMPONENTS);
    State y;
    track->DumpToArray(y);
    G4double stepLength = 2.5 * CLHEP::mm;

    test(method, equation, y, stepLength);
    test(vmethod, equation, y, stepLength);

    return 0;
}
